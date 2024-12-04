#include <renderer/device.hpp>

#include <application/window.hpp>

#include <VkBootstrap.h>
#include <SDL_vulkan.h>
#include <renderer/device.hpp>
#include <renderer/vk_initializers.hpp>

namespace fizzengine
{
    void GPUDevice::init_vulkan(Window window)
    {
        volkInitialize();
        vkb::InstanceBuilder builder;

        // make the vulkan instance, with basic debug features
        auto inst_ret = builder.set_app_name("Fizz Engine Vulkan Backend")
                            .request_validation_layers(m_use_validation_layers)
                            .use_default_debug_messenger()
                            .require_api_version(1, 3, 0)
                            .build();

        vkb::Instance vkb_inst = inst_ret.value();

        // grab the instance
        m_instance = vkb_inst.instance;
        volkLoadInstance(m_instance);
        m_debug_messenger = vkb_inst.debug_messenger;
        create_vulkan_surface(window.get_SDL_Window());

        vkb::Device vkb_device = select_device(vkb_inst);

        m_graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
        m_graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

        auto [width, height] = window.get_dimensions();
        create_swapchain(width, height);
        init_commands();
        init_sync_structures();
        spdlog::info("Vulkan instance created");
    }

    void GPUDevice::shutdown()
    {
        vkDeviceWaitIdle(m_device);
        for (int i = 0; i < k_frames_in_flight; i++)
        {
            vkDestroyCommandPool(m_device, m_frames[i].m_command_pool, nullptr);
            vkDestroyFence(m_device, m_command_buffer_executed_fence[i], nullptr);
            vkDestroySemaphore(m_device, m_render_complete_semaphore[i], nullptr);
            vkDestroySemaphore(m_device, m_image_acquired_semaphore[i], nullptr);
        }

        destroy_swapchain();
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyDevice(m_device, nullptr);

        vkb::destroy_debug_utils_messenger(m_instance, m_debug_messenger);
        vkDestroyInstance(m_instance, nullptr);
    }

    void GPUDevice::create_vulkan_surface(SDL_Window *window)
    {
        SDL_Vulkan_CreateSurface(window, m_instance, &m_surface);
    }

    vkb::Device GPUDevice::select_device(vkb::Instance vkb_inst)
    {
        // vulkan 1.3 features
        VkPhysicalDeviceVulkan13Features features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
        features.dynamicRendering = true;
        features.synchronization2 = true;

        // vulkan 1.2 features
        VkPhysicalDeviceVulkan12Features features12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
        features12.bufferDeviceAddress = true;
        features12.descriptorIndexing = true;

        // use vkbootstrap to select a gpu.
        // We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
        vkb::PhysicalDeviceSelector selector{vkb_inst};
        vkb::PhysicalDevice vkb_physical_device =
            selector
                .set_minimum_version(1, 3)
                .set_required_features_13(features)
                .set_required_features_12(features12)
                .set_surface(m_surface)
                .select()
                .value();

        vkb::DeviceBuilder device_builder{vkb_physical_device};

        vkb::Device vkb_device = device_builder.build().value();

        // Get the VkDevice handle used in the rest of a vulkan application
        m_device = vkb_device.device;
        volkLoadDevice(m_device);
        m_chosenGPU = vkb_physical_device.physical_device;
        return vkb_device;
    }

    void GPUDevice::create_swapchain(u32 width, u32 height)
    {
        vkb::SwapchainBuilder swapchainBuilder{m_chosenGPU, m_device, m_surface};

        m_swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;

        vkb::Swapchain vkbSwapchain =
            swapchainBuilder
                //.use_default_format_selection()
                .set_desired_format(
                    VkSurfaceFormatKHR{
                        .format = m_swapchain_image_format,
                        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                // use vsync present mode
                .set_desired_present_mode(m_vulkan_present_mode)
                .set_desired_extent(width, height)
                .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                .build()
                .value();

        m_swapchain_extent = vkbSwapchain.extent;
        // store swapchain and its related images
        m_swapchain = vkbSwapchain.swapchain;
        m_swapchain_images = vkbSwapchain.get_images().value();
        m_swapchain_image_views = vkbSwapchain.get_image_views().value();
    }

    void GPUDevice::destroy_swapchain()
    {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

        // destroy swapchain resources
        for (int i = 0; i < m_swapchain_image_views.size(); i++)
        {

            vkDestroyImageView(m_device, m_swapchain_image_views[i], nullptr);
        }
    }

    void GPUDevice::init_commands()
    {
        VkCommandPoolCreateInfo command_pool_info = vkinit::command_pool_create_info(m_graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        for (int i = 0; i < k_frames_in_flight; i++)
        {

            VK_CHECK(vkCreateCommandPool(m_device, &command_pool_info, nullptr, &m_frames[i].m_command_pool));

            VkCommandBufferAllocateInfo cmd_alloc_info = vkinit::command_buffer_allocate_info(m_frames[i].m_command_pool, 1);

            VK_CHECK(vkAllocateCommandBuffers(m_device, &cmd_alloc_info, &m_frames[i].m_main_command_buffer));
        }
    }

    void GPUDevice::init_sync_structures()
    {
        VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
        VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

        for (int i = 0; i < k_frames_in_flight; i++)
        {
            VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_command_buffer_executed_fence[i]));
            VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_image_acquired_semaphore[i]));
            VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_render_complete_semaphore[i]));
        }
    }

    VkCommandBuffer GPUDevice::new_frame()
    {
        VkFence *render_complete_fence = &m_command_buffer_executed_fence[m_frame_number % k_frames_in_flight];
        if (vkGetFenceStatus(m_device, *render_complete_fence) != VK_SUCCESS)
        {
            VK_CHECK(vkWaitForFences(m_device, 1, render_complete_fence, VK_TRUE, UINT64_MAX));
        }

        VK_CHECK(vkResetFences(m_device, 1, render_complete_fence));

        VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_image_acquired_semaphore[m_frame_number % k_frames_in_flight], VK_NULL_HANDLE, &m_vulkan_image_index);

        // if (result == VK_ERROR_OUT_OF_DATE_KHR)
        // {
        //     resize_swapchain();
        // }

        // Command pool reset
        VkCommandBuffer cmd = get_current_frame().m_main_command_buffer;
        VK_CHECK(vkResetCommandBuffer(cmd, 0));

        // begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
        VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // start the command buffer recording
        VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));
        return cmd;
    }

    void GPUDevice::present()
    {
        u32 frame_index = m_frame_number % k_frames_in_flight;
        VkCommandBuffer cmd = get_current_frame().m_main_command_buffer;
        VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);
        VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, m_image_acquired_semaphore[frame_index]);
        VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, m_render_complete_semaphore[frame_index]);

        VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

        VK_CHECK(vkQueueSubmit2(m_graphics_queue, 1, &submit, m_command_buffer_executed_fence[frame_index]));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.pSwapchains = &m_swapchain;
        presentInfo.swapchainCount = 1;

        presentInfo.pWaitSemaphores = &m_render_complete_semaphore[frame_index];
        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pImageIndices = &m_vulkan_image_index;

        VK_CHECK(vkQueuePresentKHR(m_graphics_queue, &presentInfo));

        // increase the number of frames drawn
        m_frame_number++;
    }
}

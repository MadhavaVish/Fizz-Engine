#include <renderer/device.hpp>

#include <application/window.hpp>

#include <SDL_vulkan.h>
#include <VkBootstrap.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <renderer/device.hpp>
#include <renderer/vk_initializers.hpp>
#include <renderer/vk_utils.hpp>

namespace fizzengine {
void GPUDevice::init_vulkan(Window window) {
    volkInitialize();
    vkb::InstanceBuilder builder;

    // make the vulkan instance, with basic debug features
    auto                 inst_ret = builder.set_app_name("Fizz Engine Vulkan Backend")
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

    vkb::Device vkb_device  = select_device(vkb_inst);

    m_graphics_queue        = vkb_device.get_queue(vkb::QueueType::graphics).value();
    m_graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

    VmaVulkanFunctions vma_vulkan_func{};
    vma_vulkan_func.vkGetInstanceProcAddr               = vkGetInstanceProcAddr;
    vma_vulkan_func.vkGetDeviceProcAddr                 = vkGetDeviceProcAddr;
    vma_vulkan_func.vkAllocateMemory                    = vkAllocateMemory;
    vma_vulkan_func.vkBindBufferMemory                  = vkBindBufferMemory;
    vma_vulkan_func.vkBindImageMemory                   = vkBindImageMemory;
    vma_vulkan_func.vkCreateBuffer                      = vkCreateBuffer;
    vma_vulkan_func.vkCreateImage                       = vkCreateImage;
    vma_vulkan_func.vkDestroyBuffer                     = vkDestroyBuffer;
    vma_vulkan_func.vkDestroyImage                      = vkDestroyImage;
    vma_vulkan_func.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
    vma_vulkan_func.vkFreeMemory                        = vkFreeMemory;
    vma_vulkan_func.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
    vma_vulkan_func.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
    vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vma_vulkan_func.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
    vma_vulkan_func.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
    vma_vulkan_func.vkMapMemory                         = vkMapMemory;
    vma_vulkan_func.vkUnmapMemory                       = vkUnmapMemory;
    vma_vulkan_func.vkCmdCopyBuffer                     = vkCmdCopyBuffer;

    VmaAllocatorCreateInfo allocator_info               = {};
    allocator_info.physicalDevice                       = m_chosen_GPU;
    allocator_info.device                               = m_device;
    allocator_info.instance                             = m_instance;
    allocator_info.flags            = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocator_info.pVulkanFunctions = &vma_vulkan_func;

    vmaCreateAllocator(&allocator_info, &m_vma_allocator);

    m_main_deletion_queue.push_function([&]() { vmaDestroyAllocator(m_vma_allocator); });

    auto [width, height] = window.get_dimensions();
    create_swapchain(width, height);
    create_draw_target(width, height);

    init_commands();

    init_sync_structures();
    init_descriptors();
    init_pipelines();

    spdlog::info("Vulkan instance created");
}

void GPUDevice::shutdown() {
    vkDeviceWaitIdle(m_device);
    for (int i = 0; i < k_frames_in_flight; i++) {
        vkDestroyCommandPool(m_device, m_frames[i].m_command_pool, nullptr);
        vkDestroyFence(m_device, m_command_buffer_executed_fence[i], nullptr);
        vkDestroySemaphore(m_device, m_render_complete_semaphore[i], nullptr);
        vkDestroySemaphore(m_device, m_image_acquired_semaphore[i], nullptr);

        m_frames[i].m_deletion_queue.flush();
    }

    m_main_deletion_queue.flush();

    destroy_swapchain();
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDevice(m_device, nullptr);

    vkb::destroy_debug_utils_messenger(m_instance, m_debug_messenger);
    vkDestroyInstance(m_instance, nullptr);
}

void GPUDevice::create_vulkan_surface(SDL_Window* window) {
    SDL_Vulkan_CreateSurface(window, m_instance, &m_surface);
}

vkb::Device GPUDevice::select_device(vkb::Instance vkb_inst) {
    // vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    features.dynamicRendering = true;
    features.synchronization2 = true;

    // vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features12.bufferDeviceAddress             = true;
    features12.descriptorIndexing              = true;
    features12.descriptorBindingPartiallyBound = true;
    features12.runtimeDescriptorArray          = true;

    vkb::PhysicalDeviceSelector selector{vkb_inst};
    vkb::PhysicalDevice         vkb_physical_device = selector.set_minimum_version(1, 3)
                                                  .set_required_features_13(features)
                                                  .set_required_features_12(features12)
                                                  .set_surface(m_surface)
                                                  .select()
                                                  .value();

    // Already available in 1.3 but imgui needs it because it needs the extension version to make
    // the multiple viewports work
    vkb_physical_device.enable_extension_if_present("VK_KHR_dynamic_rendering");
    vkb::DeviceBuilder device_builder{vkb_physical_device};

    vkb::Device        vkb_device = device_builder.build().value();

    m_device                      = vkb_device.device;
    volkLoadDevice(m_device);

    m_chosen_GPU = vkb_physical_device.physical_device;
    return vkb_device;
}

void GPUDevice::create_draw_target(u32 width, u32 height) {
    VkExtent3D drawImageExtent = {width, height, 1};

    // hardcoding the draw format to 32 bit float
    m_draw_image.m_format = VK_FORMAT_R16G16B16A16_SFLOAT;
    m_draw_image.width    = width;
    m_draw_image.height   = height;

    VkImageUsageFlags draw_image_usage{};
    draw_image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    draw_image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    draw_image_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    draw_image_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    draw_image_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageCreateInfo rimg_info =
        vkinit::image_create_info(m_draw_image.m_format, draw_image_usage,
                                  VkExtent3D{.width = width, .height = height, .depth = 1});

    // for the draw image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // allocate and create the image
    vmaCreateImage(m_vma_allocator, &rimg_info, &rimg_allocinfo, &m_draw_image.m_image,
                   &m_draw_image.m_vma_allocation, nullptr);

    // build a image-view for the draw image to use for rendering
    VkImageViewCreateInfo rview_info = vkinit::imageview_create_info(
        m_draw_image.m_format, m_draw_image.m_image, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(m_device, &rview_info, nullptr, &m_draw_image.m_image_view));

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType      = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter  = VK_FILTER_LINEAR;
    sampler_info.minFilter  = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU =
        VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
    sampler_info.addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.minLod        = -1000;
    sampler_info.maxLod        = 1000;
    sampler_info.maxAnisotropy = 1.0f;
    VK_CHECK(vkCreateSampler(m_device, &sampler_info, nullptr, &m_draw_image.m_sampler));

    // add to deletion queues
    m_main_deletion_queue.push_function([=, this]() {
        vkDestroySampler(m_device, m_draw_image.m_sampler, nullptr);
        vkDestroyImageView(m_device, m_draw_image.m_image_view, nullptr);
        vmaDestroyImage(m_vma_allocator, m_draw_image.m_image, m_draw_image.m_vma_allocation);
    });
}

void GPUDevice::create_swapchain(u32 width, u32 height) {
    vkb::SwapchainBuilder swapchainBuilder{m_chosen_GPU, m_device, m_surface};

    m_swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain =
        swapchainBuilder
            //.use_default_format_selection()
            .set_desired_format(VkSurfaceFormatKHR{.format     = m_swapchain_image_format,
                                                   .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
            // use vsync present mode
            .set_desired_present_mode(m_vulkan_present_mode)
            .set_desired_extent(width, height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build()
            .value();

    m_swapchain_extent = vkbSwapchain.extent;
    // store swapchain and its related images
    m_swapchain             = vkbSwapchain.swapchain;
    m_swapchain_images      = vkbSwapchain.get_images().value();
    m_swapchain_image_views = vkbSwapchain.get_image_views().value();
}

void GPUDevice::destroy_swapchain() {
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    // destroy swapchain resources
    for (int i = 0; i < m_swapchain_image_views.size(); i++) {

        vkDestroyImageView(m_device, m_swapchain_image_views[i], nullptr);
    }
}

void GPUDevice::init_commands() {
    VkCommandPoolCreateInfo command_pool_info = vkinit::command_pool_create_info(
        m_graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i = 0; i < k_frames_in_flight; i++) {

        VK_CHECK(vkCreateCommandPool(m_device, &command_pool_info, nullptr,
                                     &m_frames[i].m_command_pool));

        VkCommandBufferAllocateInfo cmd_alloc_info =
            vkinit::command_buffer_allocate_info(m_frames[i].m_command_pool, 1);

        VK_CHECK(vkAllocateCommandBuffers(m_device, &cmd_alloc_info,
                                          &m_frames[i].m_main_command_buffer));
    }
}

void GPUDevice::init_sync_structures() {
    VkFenceCreateInfo     fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

    for (int i = 0; i < k_frames_in_flight; i++) {
        VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr,
                               &m_command_buffer_executed_fence[i]));
        VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr,
                                   &m_image_acquired_semaphore[i]));
        VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr,
                                   &m_render_complete_semaphore[i]));
    }
}

void GPUDevice::init_descriptors() {
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
        {.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .ratio = 1},
    };
    m_global_descriptor_allocator.init_pool(m_device, 10, sizes);

    {
        DescriptorLayoutBuilder builder;
        builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        m_draw_image_descriptor_layout = builder.build(m_device, VK_SHADER_STAGE_COMPUTE_BIT);
    }

    m_draw_image_descriptors =
        m_global_descriptor_allocator.allocate(m_device, m_draw_image_descriptor_layout);

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout                 = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView                   = m_draw_image.m_image_view;

    VkWriteDescriptorSet drawImageWrite = {};
    drawImageWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    drawImageWrite.pNext                = nullptr;

    drawImageWrite.dstBinding           = 0;
    drawImageWrite.dstSet               = m_draw_image_descriptors;
    drawImageWrite.descriptorCount      = 1;
    drawImageWrite.descriptorType       = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    drawImageWrite.pImageInfo           = &imgInfo;

    vkUpdateDescriptorSets(m_device, 1, &drawImageWrite, 0, nullptr);

    // make sure both the descriptor allocator and the new layout get cleaned up properly
    m_main_deletion_queue.push_function([&]() {
        m_global_descriptor_allocator.destroy_pool(m_device);
        vkDestroyDescriptorSetLayout(m_device, m_draw_image_descriptor_layout, nullptr);
    });
}

void GPUDevice::init_pipelines() {
    init_grad_pipeline();
}

void GPUDevice::init_grad_pipeline() {
    VkPipelineLayoutCreateInfo computeLayout{};
    computeLayout.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayout.pNext          = nullptr;
    computeLayout.pSetLayouts    = &m_draw_image_descriptor_layout;
    computeLayout.setLayoutCount = 1;

    VK_CHECK(vkCreatePipelineLayout(m_device, &computeLayout, nullptr, &m_grad_pipeline_layout));
    VkShaderModule computeDrawShader = vkutil::CompileSlangShader(
        m_device, "../../shaders/gradient.comp.slang", "main", VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineShaderStageCreateInfo stageinfo{};
    stageinfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageinfo.pNext  = nullptr;
    stageinfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
    stageinfo.module = computeDrawShader;
    stageinfo.pName  = "main";

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext  = nullptr;
    computePipelineCreateInfo.layout = m_grad_pipeline_layout;
    computePipelineCreateInfo.stage  = stageinfo;

    VK_CHECK(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo,
                                      nullptr, &m_grad_pipeline));

    vkDestroyShaderModule(m_device, computeDrawShader, nullptr);

    m_main_deletion_queue.push_function([&]() {
        vkDestroyPipelineLayout(m_device, m_grad_pipeline_layout, nullptr);
        vkDestroyPipeline(m_device, m_grad_pipeline, nullptr);
    });
}

VkCommandBuffer GPUDevice::new_frame() {
    VkFence* render_complete_fence =
        &m_command_buffer_executed_fence[m_frame_number % k_frames_in_flight];
    if (vkGetFenceStatus(m_device, *render_complete_fence) != VK_SUCCESS) {
        VK_CHECK(vkWaitForFences(m_device, 1, render_complete_fence, VK_TRUE, UINT64_MAX));
    }
    get_current_frame().m_deletion_queue.flush();

    VK_CHECK(vkResetFences(m_device, 1, render_complete_fence));

    VkResult result =
        vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
                              m_image_acquired_semaphore[m_frame_number % k_frames_in_flight],
                              VK_NULL_HANDLE, &m_vulkan_image_index);

    // if (result == VK_ERROR_OUT_OF_DATE_KHR)
    // {
    //     resize_swapchain();
    // }

    // Command pool reset

    m_draw_extent.width  = m_draw_image.width;
    m_draw_extent.height = m_draw_image.height;
    VkCommandBuffer cmd  = get_current_frame().m_main_command_buffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    // begin the command buffer recording. We will use this command buffer exactly once, so we want
    // to let vulkan know that
    VkCommandBufferBeginInfo cmd_begin_info =
        vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // start the command buffer recording
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));
    return cmd;
}

void GPUDevice::present() {
    u32                       frame_index = m_frame_number % k_frames_in_flight;
    VkCommandBuffer           cmd         = get_current_frame().m_main_command_buffer;
    VkCommandBufferSubmitInfo cmdinfo     = vkinit::command_buffer_submit_info(cmd);
    VkSemaphoreSubmitInfo     waitInfo =
        vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                                      m_image_acquired_semaphore[frame_index]);
    VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_submit_info(
        VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, m_render_complete_semaphore[frame_index]);

    VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

    VK_CHECK(
        vkQueueSubmit2(m_graphics_queue, 1, &submit, m_command_buffer_executed_fence[frame_index]));

    VkPresentInfoKHR presentInfo   = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.pSwapchains        = &m_swapchain;
    presentInfo.swapchainCount     = 1;

    presentInfo.pWaitSemaphores    = &m_render_complete_semaphore[frame_index];
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices      = &m_vulkan_image_index;

    VK_CHECK(vkQueuePresentKHR(m_graphics_queue, &presentInfo));

    // increase the number of frames drawn
    m_frame_number++;
}
} // namespace fizzengine

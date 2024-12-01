#include <renderer/device.hpp>

#include <application/window.hpp>

#include <VkBootstrap.h>
#include <SDL_vulkan.h>

namespace fizzengine
{
    void GPUDevice::init_vulkan(Window window)
    {
        volkInitialize();
        vkb::InstanceBuilder builder;

        // make the vulkan instance, with basic debug features
        auto inst_ret = builder.set_app_name("Example Vulkan Application")
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

        select_device(vkb_inst);
        auto [width, height] = window.get_dimensions();
        create_swapchain(width, height);

        // spdlog::info("Vulkan instance created");
    }

    void GPUDevice::shutdown()
    {
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

    void GPUDevice::select_device(vkb::Instance vkb_inst)
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
        vkb::PhysicalDevice physicalDevice =
            selector
                .set_minimum_version(1, 3)
                .set_required_features_13(features)
                .set_required_features_12(features12)
                .set_surface(m_surface)
                .select()
                .value();

        vkb::DeviceBuilder deviceBuilder{physicalDevice};

        vkb::Device vkbDevice = deviceBuilder.build().value();

        // Get the VkDevice handle used in the rest of a vulkan application
        m_device = vkbDevice.device;
        volkLoadDevice(m_device);
        m_chosenGPU = physicalDevice.physical_device;
    }

    void GPUDevice::create_swapchain(u32 width, u32 height)
    {
        vkb::SwapchainBuilder swapchainBuilder{m_chosenGPU, m_device, m_surface};

        m_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

        vkb::Swapchain vkbSwapchain =
            swapchainBuilder
                //.use_default_format_selection()
                .set_desired_format(VkSurfaceFormatKHR{.format = m_swapchainImageFormat,
                                                       .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                // use vsync present mode
                .set_desired_present_mode(m_vulkan_present_mode)
                .set_desired_extent(width, height)
                .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                .build()
                .value();

        m_swapchainExtent = vkbSwapchain.extent;
        // store swapchain and its related images
        m_swapchain = vkbSwapchain.swapchain;
        m_swapchainImages = vkbSwapchain.get_images().value();
        m_swapchainImageViews = vkbSwapchain.get_image_views().value();
    }

    void GPUDevice::destroy_swapchain()
    {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

        // destroy swapchain resources
        for (int i = 0; i < m_swapchainImageViews.size(); i++)
        {

            vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
        }
    }
}

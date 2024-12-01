#pragma once
#include <foundation/platform.hpp>
#include <renderer/vk_types.hpp>

namespace vkb
{
    class Instance;
}
class SDL_Window;
namespace fizzengine
{
    class Window;

    struct GPUDevice
    {
        bool m_use_validation_layers{false};
        VkInstance m_instance;                      // Vulkan library handle
        VkDebugUtilsMessengerEXT m_debug_messenger; // Vulkan debug output handle
        VkPhysicalDevice m_chosenGPU;               // GPU chosen as the default device
        VkDevice m_device;
        VkSurfaceKHR m_surface;

        VkSwapchainKHR m_swapchain;
        VkPresentModeKHR m_vulkan_present_mode{VK_PRESENT_MODE_FIFO_KHR};
        VkFormat m_swapchainImageFormat;

        std::vector<VkImage> m_swapchainImages;
        std::vector<VkImageView> m_swapchainImageViews;
        VkExtent2D m_swapchainExtent;

        void init_vulkan(Window window);
        void shutdown();

    private:
        void create_vulkan_surface(SDL_Window *window);
        void select_device(vkb::Instance vkb_inst);
        void create_swapchain(u32 width, u32 height);
        void destroy_swapchain();
    };
}
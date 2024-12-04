#pragma once
#include <foundation/platform.hpp>
#include <renderer/vk_types.hpp>

namespace vkb
{
    class Instance;
    class Device;
}
class SDL_Window;

constexpr unsigned int k_frames_in_flight = 10;

namespace fizzengine
{
    class Window;
    struct FrameData
    {
        VkCommandPool m_command_pool;
        VkCommandBuffer m_main_command_buffer;
    };
    struct GPUDevice
    {
        bool m_use_validation_layers{true};
        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debug_messenger;
        VkPhysicalDevice m_chosenGPU;
        VkDevice m_device;
        VkSurfaceKHR m_surface; // Drawing Surface TODO: make this optional for headless

        VkSwapchainKHR m_swapchain;
        VkPresentModeKHR m_vulkan_present_mode{VK_PRESENT_MODE_IMMEDIATE_KHR};
        VkFormat m_swapchain_image_format;

        std::vector<VkImage> m_swapchain_images;
        std::vector<VkImageView> m_swapchain_image_views;
        VkExtent2D m_swapchain_extent;

        u32 m_frame_number{0};
        FrameData m_frames[k_frames_in_flight];

        // Per frame sync
        u32 m_vulkan_image_index;
        VkSemaphore m_image_acquired_semaphore[k_frames_in_flight];
        VkSemaphore m_render_complete_semaphore[k_frames_in_flight];
        VkFence m_command_buffer_executed_fence[k_frames_in_flight];

        VkQueue m_graphics_queue;
        uint32_t m_graphics_queue_family;

        void init_vulkan(Window window);
        void shutdown();

        FrameData &get_current_frame()
        {
            return m_frames[m_frame_number % k_frames_in_flight];
        };
        VkCommandBuffer new_frame();
        void present();

    private:
        vkb::Device select_device(vkb::Instance vkb_inst);

        void create_vulkan_surface(SDL_Window *window);

        void create_swapchain(u32 width, u32 height);
        void destroy_swapchain();

        void init_commands();

        void init_sync_structures();
    };
}
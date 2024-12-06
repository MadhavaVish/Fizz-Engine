#pragma once

#include <renderer/vk_types.hpp>
#include <renderer/gpu_resources.hpp>

namespace vkb
{
    class Instance;
    class Device;
}
class SDL_Window;

constexpr unsigned int k_frames_in_flight = 3;

namespace fizzengine
{
    class Window;

    struct DeletionQueue
    {
        std::deque<std::function<void()>> deletors;

        void push_function(std::function<void()> &&function)
        {
            deletors.push_back(function);
        }

        void flush()
        {
            // reverse iterate the deletion queue to execute all the functions
            for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
            {
                (*it)(); // call functors
            }

            deletors.clear();
        }
    };

    struct FrameData
    {
        VkCommandPool m_command_pool;
        VkCommandBuffer m_main_command_buffer;

        DeletionQueue m_deletion_queue;
    };

    struct GPUDevice
    {
        bool m_use_validation_layers{true};
        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debug_messenger;
        VkPhysicalDevice m_chosen_GPU;
        VkDevice m_device;
        VkSurfaceKHR m_surface; // Drawing Surface TODO: make this optional for headless

        VmaAllocator m_vma_allocator;

        VkSwapchainKHR m_swapchain;
        VkPresentModeKHR m_vulkan_present_mode{VK_PRESENT_MODE_IMMEDIATE_KHR};
        VkFormat m_swapchain_image_format;

        Texture m_draw_image;
        VkExtent2D m_draw_extent;

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

        DeletionQueue m_main_deletion_queue;

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

        void create_draw_target(u32 width, u32 height);

        void create_swapchain(u32 width, u32 height);
        void destroy_swapchain();

        void init_commands();

        void init_sync_structures();
    };
}
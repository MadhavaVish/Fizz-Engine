#pragma once

#include <renderer/gpu_resources.hpp>
#include <renderer/vk_types.hpp>

namespace vkb {
class Instance;
class Device;
} // namespace vkb
class SDL_Window;

constexpr unsigned int k_frames_in_flight = 3;

namespace fizzengine {
class Window;

// clang-format off
struct DeletionQueue {

    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)(); // call functors
        }

        deletors.clear();
    }
};
// clang-format on

struct FrameData {
    VkCommandPool   m_command_pool;
    VkCommandBuffer m_main_command_buffer;

    DeletionQueue   m_deletion_queue;
};

struct GPUDevice {
    bool                     m_use_validation_layers{true};
    VkInstance               m_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger;
    VkPhysicalDevice         m_chosen_GPU;
    VkDevice                 m_device;
    VkSurfaceKHR             m_surface;
    VmaAllocator             m_vma_allocator;

    VkSwapchainKHR           m_swapchain;
    VkPresentModeKHR         m_vulkan_present_mode;
    VkFormat                 m_swapchain_image_format;

    Texture                  m_draw_image;
    VkExtent2D               m_draw_extent;

    std::vector<VkImage>     m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
    VkExtent2D               m_swapchain_extent;

    u32                      m_frame_number;
    u32                      m_vulkan_image_index;

    FrameData                m_frames[k_frames_in_flight];

    // Per frame sync
    VkSemaphore              m_image_acquired_semaphore[k_frames_in_flight];
    VkSemaphore              m_render_complete_semaphore[k_frames_in_flight];
    VkFence                  m_command_buffer_executed_fence[k_frames_in_flight];

    VkQueue                  m_graphics_queue;
    uint32_t                 m_graphics_queue_family;

    DescriptorAllocator      m_global_descriptor_allocator;

    VkDescriptorSet          m_draw_image_descriptors;
    VkDescriptorSetLayout    m_draw_image_descriptor_layout;

    VkPipeline               m_grad_pipeline;
    VkPipelineLayout         m_grad_pipeline_layout;

    DeletionQueue            m_main_deletion_queue;

    void                     init_vulkan(Window window);
    void                     shutdown();

    FrameData&               get_current_frame() {
        return m_frames[m_frame_number % k_frames_in_flight];
    }

    VkImage& get_current_swapchain_image() {
        return m_swapchain_images[m_vulkan_image_index];
    }
    VkImageView& get_current_swapchain_image_view() {
        return m_swapchain_image_views[m_vulkan_image_index];
    }

    VkCommandBuffer new_frame();
    void            present();

  private:
    vkb::Device select_device(vkb::Instance vkb_inst);

    void        create_vulkan_surface(SDL_Window* window);

    void        create_draw_target(u32 width, u32 height);

    void        create_swapchain(u32 width, u32 height);
    void        destroy_swapchain();

    void        init_commands();

    void        init_sync_structures();

    void        init_descriptors();

    void        init_pipelines();
    void        init_grad_pipeline();
};
} // namespace fizzengine
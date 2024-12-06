#include "engine.hpp"

#include <spdlog/spdlog.h>

#include <renderer/renderer.hpp>
#include <renderer/vk_utils.hpp>
#include <renderer/vk_initializers.hpp>

namespace fizzengine
{
    void FizzEngine::init()
    {
        m_window.init();
        m_gpu.init_vulkan(m_window);
        m_renderer.init();
        spdlog::info("Fizz Engine Initialized");
        is_initialized = true;
    }

    void FizzEngine::shutdown()
    {
        m_gpu.shutdown();
        m_window.shutdown();

        spdlog::info("Fizz Engine Closed");
    }

    void FizzEngine::update()
    {
    }

    void FizzEngine::render()
    {
        VkCommandBuffer cmd = m_gpu.new_frame();

        {
            vkutil::transition_image(cmd, m_gpu.m_draw_image.m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

            // make a clear-color from frame number. This will flash with a 120 frame period.
            VkClearColorValue clearValue;
            float flash = std::abs(std::sin(m_gpu.m_frame_number / 120.f));
            clearValue = {{0.0f, 0.0f, flash, 1.0f}};

            VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

            // clear image
            vkCmdClearColorImage(cmd, m_gpu.m_draw_image.m_image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
            // make the draw image into presentable mode
            vkutil::transition_image(cmd, m_gpu.m_draw_image.m_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            vkutil::transition_image(cmd, m_gpu.m_swapchain_images[m_gpu.m_vulkan_image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            vkutil::copy_image_to_image(cmd, m_gpu.m_draw_image.m_image, m_gpu.m_swapchain_images[m_gpu.m_vulkan_image_index], m_gpu.m_draw_extent, m_gpu.m_swapchain_extent);

            // set swapchain image layout to Present so we can show it on the screen
            vkutil::transition_image(cmd, m_gpu.m_swapchain_images[m_gpu.m_vulkan_image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            // finalize the command buffer (we can no longer add commands, but it can now be executed)
            VK_CHECK(vkEndCommandBuffer(cmd));
        }
        m_gpu.present();
    }

    void FizzEngine::run()
    {
        bool b_quit = false;
        while (!b_quit)
        {
            m_window.handle_events(b_quit);

            render();
        }
    }

}
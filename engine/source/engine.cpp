#include "engine.hpp"

#include <spdlog/spdlog.h>

#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>
#include <foundation/allocators.hpp>
#include <foundation/resource_pool.hpp>
#include <imgui.h>
#include <renderer/renderer.hpp>
#include <renderer/vk_initializers.hpp>
#include <renderer/vk_utils.hpp>

namespace fizzengine {

void FizzEngine::init() {
    m_window.init();
    m_gpu.init_vulkan(m_window);
    init_imgui();
    img = ImGui_ImplVulkan_AddTexture(m_gpu.m_draw_image.m_sampler, m_gpu.m_draw_image.m_image_view,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    spdlog::info("Fizz Engine Initialized");
    is_initialized = true;
}

void FizzEngine::shutdown() {
    ImGui_ImplVulkan_RemoveTexture(img);
    m_gpu.shutdown();
    m_window.shutdown();

    spdlog::info("Fizz Engine Closed");
}

void FizzEngine::update() {
}

void FizzEngine::render() {
    VkCommandBuffer cmd = m_gpu.new_frame();

    {
        VkImage draw_image = m_gpu.m_draw_image.m_image;
        vkutil::transition_image(cmd, draw_image, VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_GENERAL);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_gpu.m_grad_pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_gpu.m_grad_pipeline_layout,
                                0, 1, &m_gpu.m_draw_image_descriptors, 0, nullptr);

        vkCmdDispatch(cmd, std::ceil(m_gpu.m_draw_extent.width / 16.0),
                      std::ceil(m_gpu.m_draw_extent.height / 16.0), 1);

        // make the draw image into presentable mode
        vkutil::transition_image(cmd, draw_image, VK_IMAGE_LAYOUT_GENERAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        // vkutil::transition_image(cmd, m_gpu.get_current_swapchain_image(),
        // VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // vkutil::copy_image_to_image(cmd, draw_image, m_gpu.get_current_swapchain_image(),
        // m_gpu.m_draw_extent, m_gpu.m_swapchain_extent);

        vkutil::transition_image(cmd, m_gpu.get_current_swapchain_image(),
                                 VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        draw_imgui(cmd, m_gpu.get_current_swapchain_image_view());
        // set swapchain image layout to Present so we can show it on the screen
        vkutil::transition_image(cmd, m_gpu.get_current_swapchain_image(),
                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // finalize the command buffer (we can no longer add commands, but it can now be executed)
        VK_CHECK(vkEndCommandBuffer(cmd));
    }
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
    m_gpu.present();
}

void FizzEngine::run() {
    bool b_quit = false;
    while (!b_quit) {
        m_window.handle_events(b_quit);
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // some imgui UI to test
        ImGui::DockSpaceOverViewport();
        bool show_viewport = true;
        ImGui::Begin("Viewport");
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        ImGui::Image((ImTextureID)img, ImVec2{viewportPanelSize.x, viewportPanelSize.y});
        ImGui::End();
        // some imgui UI to test
        ImGui::ShowDemoWindow();
        // make imgui calculate internal draw structures
        ImGui::Render();
        // our draw function

        render();
    }
}

void FizzEngine::init_imgui() {
    VkDescriptorPoolSize       pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                               {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                               {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                               {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                               {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                               {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                               {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info    = {};
    pool_info.sType                         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags                         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets                       = 1000;
    pool_info.poolSizeCount                 = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes                    = pool_sizes;

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(m_gpu.m_device, &pool_info, nullptr, &imguiPool));

    // 2: initialize imgui library

    // this initializes the core structures of imgui
    ImGui::CreateContext();

    // this initializes imgui for SDL
    ImGui_ImplSDL2_InitForVulkan(m_window.get_SDL_Window());

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = m_gpu.m_instance;
    init_info.PhysicalDevice            = m_gpu.m_chosen_GPU;
    init_info.Device                    = m_gpu.m_device;
    init_info.Queue                     = m_gpu.m_graphics_queue;
    init_info.DescriptorPool            = imguiPool;
    init_info.MinImageCount             = 3;
    init_info.ImageCount                = 3;
    init_info.UseDynamicRendering       = true;

    // dynamic rendering parameters for imgui to use
    init_info.PipelineRenderingCreateInfo                         = {.sType =
                                                                         VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount    = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_gpu.m_swapchain_image_format;

    init_info.MSAASamples                                         = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);

    ImGui_ImplVulkan_CreateFontsTexture();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    // add the destroy the imgui created structures
    m_gpu.m_main_deletion_queue.push_function([=, this]() {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(m_gpu.m_device, imguiPool, nullptr);
    });
}

void FizzEngine::draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView) {
    VkRenderingAttachmentInfo colorAttachment =
        vkinit::attachment_info(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo =
        vkinit::rendering_info(m_gpu.m_swapchain_extent, &colorAttachment, nullptr);

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}

} // namespace fizzengine

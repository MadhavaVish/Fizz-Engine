#pragma once

#include <foundation/platform.hpp>

#include <application/window.hpp>
#include <renderer/renderer.hpp>

namespace fizzengine {

class FizzEngine {
  public:
    void            init();
    void            shutdown();
    void            update();
    void            render();
    void            run();

    bool            is_initialized{false};

    Window          m_window{"Fizz Engine", 1280, 720};
    GPUDevice       m_gpu;
    Renderer        m_renderer;
    VkDescriptorSet img;

  private:
    void init_imgui();
    void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);
};

} // namespace fizzengine

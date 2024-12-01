#include "application/window.hpp"

#include <spdlog/spdlog.h>

namespace fizzengine
{

    Window::Window(const std::string &title, int width, int height)
        : m_title(title), m_width(width), m_height(height) {}

    bool Window::init()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            spdlog::error("Failed to initialize SDL");
            return false;
        }
        p_window_handle = SDL_CreateWindow(
            m_title.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            m_width,
            m_height,
            SDL_WINDOW_VULKAN);

        if (!p_window_handle)
        {
            spdlog::error("Failed to create SDL Window");
            return false;
        }

        return true;
    }
    void Window::shutdown()
    {
        if (p_window_handle)
            SDL_DestroyWindow(p_window_handle);
        SDL_Quit();
    }
    void Window::handle_events(bool &quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }
    }
}
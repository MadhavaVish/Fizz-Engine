#pragma once

#include <SDL.h>
#include <string>

#include <foundation/platform.hpp>

namespace fizzengine
{
    struct WindowDims
    {
        u32 width;
        u32 height;
    };
    class FIZZENGINE_API Window
    {
    public:
        Window(const std::string &title, i32 width, i32 height);

        bool init();
        void shutdown();
        SDL_Window *get_SDL_Window() const { return p_window_handle; }

        WindowDims get_dimensions() const
        {
            return WindowDims{m_width, m_height};
            // width = m_width;
            // height = m_height;
        }
        void handle_events(bool &quit);

    private:
        std::string m_title;
        u32 m_width{0};
        u32 m_height{0};
        SDL_Window *p_window_handle = nullptr;
    };

}
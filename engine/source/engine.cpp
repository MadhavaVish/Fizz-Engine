#include "engine.hpp"

#include <spdlog/spdlog.h>

#include <renderer/renderer.hpp>

namespace fizzengine
{
    void FizzEngine::init()
    {
        m_window.init();
        m_device.init_vulkan(m_window);
        m_renderer.init();
        spdlog::info("Fizz Engine Initialized");
        is_initialized = true;
    }

    void FizzEngine::shutdown()
    {
        m_device.shutdown();
        m_window.shutdown();

        spdlog::info("Fizz Engine Closed");
    }

    void FizzEngine::update()
    {
    }

    void FizzEngine::render()
    {
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
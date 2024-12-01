#pragma once

#include <foundation/platform.hpp>

#include <application/window.hpp>
#include <renderer/renderer.hpp>

namespace fizzengine
{
	class FIZZENGINE_API FizzEngine
	{
	public:
		void init();
		void shutdown();
		void update();
		void render();
		void run();

		bool is_initialized{false};

		Window m_window{"Fizz Engine", 1280, 720};
		GPUDevice m_device;
		Renderer m_renderer;
		u32 m_frame_number{0};
	};
}

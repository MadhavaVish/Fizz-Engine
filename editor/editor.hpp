#pragma once

#include <string>

#include <foundation/platform.hpp>
#include <application/window.hpp>
#include <engine.hpp>

namespace fizzeditor
{

	class FizzEditor
	{
	public:
		FizzEditor();
		~FizzEditor() {};

		void init();
		void shutdown();
		void run();

	private:
		std::string m_app_name;
		fizzengine::FizzEngine m_engine;
	};
}

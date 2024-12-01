#pragma once
// #include <renderer/vk_types.hpp>

// #include <vma/vk_mem_alloc.h>
// #include <string>
// #include <vector>
#include <foundation/platform.hpp>
#include <renderer/device.hpp>

namespace fizzengine
{
	class FIZZENGINE_API Renderer
	{
	public:
		void init();
		void shutdown();
	};
}

#pragma once

#include <foundation/platform.hpp>
#include <renderer/device.hpp>

namespace fizzengine {
class FIZZENGINE_API Renderer {
  public:
    void init();
    void shutdown();
};
} // namespace fizzengine

#pragma once

#include <string>

#include <application/window.hpp>
#include <engine.hpp>
#include <foundation/platform.hpp>

namespace fizzeditor {

class FizzEditor {
  public:
    FizzEditor();
    ~FizzEditor() {};

    void init();
    void shutdown();
    void run();

  private:
    std::string            m_app_name;
    fizzengine::FizzEngine m_engine;
};

} // namespace fizzeditor

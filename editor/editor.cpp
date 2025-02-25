#include "editor.hpp"

namespace fizzeditor {

FizzEditor::FizzEditor() : m_app_name("Fizz Editor") {
}

void FizzEditor::init() {
    m_engine.init();
}
void FizzEditor::shutdown() {
    m_engine.shutdown();
}
void FizzEditor::run() {
    m_engine.run();
}

} // namespace fizzeditor
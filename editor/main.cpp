#define SDL_MAIN_HANDLED

#include "editor.hpp"
#include <engine.hpp>

int main(int argc, char* argv[]) {
    fizzeditor::FizzEditor editor;
    editor.init();
    editor.run();
    editor.shutdown();
    return 0;
}
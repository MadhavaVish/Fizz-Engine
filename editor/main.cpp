#define SDL_MAIN_HANDLED

#include <foundation/platform.hpp>
#include <engine.hpp>
#include "editor.hpp"

int main(int argc, char *argv[])
{
	fizzeditor::FizzEditor editor;
	editor.init();
	editor.run();
	editor.shutdown();

	return 0;
}
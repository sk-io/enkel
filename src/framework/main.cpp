#include "framework.h"

#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Please call this executable with a path to the starting script", NULL);
		return 1;
	}

	run_framework(argv[1]);
	return 0;
}
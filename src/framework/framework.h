#pragma once

#include "image.h"

#include <enkel/interpreter.h>
#include <enkel/value.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <string>
#include <stdint.h>
#include <vector>

struct Framework {
	SDL_Window* window;
	SDL_Renderer* renderer;

	int width = 800, height = 600;
	int scale = 1;
	std::string title = "enkel framework";

	Interpreter interp;
	Value init_func;
	Value update_func;
	Value draw_func;

	std::vector<Image> images;
};

extern Framework fw;

void run_framework();
void framework_error(const std::string& msg = "");

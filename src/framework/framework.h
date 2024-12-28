#pragma once

#include "image.h"

#include <enkel/interpreter.h>
#include <enkel/value.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <string>
#include <stdint.h>
#include <vector>
#include <unordered_map>

struct Framework {
	SDL_Window* window;
	SDL_Renderer* renderer;

	int width = 800, height = 600;
	int scale = 1;
	float framerate = 60; // <= 0 means uncapped
	std::string title = "enkel framework";

	Interpreter interp;
	Value init_func;
	Value update_func;
	Value draw_func;

	std::vector<Image> images;
	std::unordered_map<int32_t, bool> keyboard_state;
	std::vector<std::string> script_paths;
};

extern Framework fw;

void run_framework(const std::string& script_path);
void framework_error(const std::string& msg = "", const Source_Info* info = nullptr);
bool is_key_down(SDL_Keycode keycode);

#pragma once

struct Framework {
	SDL_Window* window;
	SDL_Renderer* renderer;

	int width = 800, height = 600;
	int scale = 1;
	std::string title = "enkel framework";

	uint32_t color = 0xFF00FF;
};

extern Framework framework;

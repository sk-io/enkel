#pragma once

#include <SDL2/SDL.h>
#include <string>

class Image {
public:
	Image() {}
	Image(const std::string& path);

	SDL_Texture* get_texture() const { return tex; }
	const int get_width() const { return width; }
	const int get_height() const { return height; }

private:
	SDL_Texture* tex = nullptr;
	int width = -1, height = -1;
};

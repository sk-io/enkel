#include "image.h"
#include "framework.h"

#include <SDL2/SDL_image.h>

Image::Image(const std::string& path) {
	SDL_Surface* surf = IMG_Load(path.c_str());
	if (surf == NULL) {
		framework_error("failed to load image: " + path);
	}

	width = surf->w;
	height = surf->h;

	tex = SDL_CreateTextureFromSurface(fw.renderer, surf);
	if (tex == NULL) {
		framework_error("failed to convert surface to texture: " + path);
	}

	SDL_FreeSurface(surf);
}

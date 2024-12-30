#include "graphics.h"

#include "framework.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

void Graphics::init() {
	fw.renderer = SDL_CreateRenderer(fw.window, -1, SDL_RENDERER_ACCELERATED);
}

void Graphics::set_color(uint32_t _color) {
	color = _color;
	int r = _color >> 16 & 255;
	int g = _color >> 8 & 255;
	int b = _color & 255;
	SDL_SetRenderDrawColor(fw.renderer, r, g, b, 255);
}

void Graphics::clear() const {
	SDL_RenderClear(fw.renderer);
}

void Graphics::swap_buffers() {
	SDL_RenderPresent(fw.renderer);
}

void Graphics::fill_rect(float x, float y, float w, float h) {
	const SDL_Rect rect = {x, y, w, h};
	SDL_RenderFillRect(fw.renderer, &rect);
}

void Graphics::draw_img(const Image& img, float x, float y, float w, float h) {
	const SDL_Rect target = {x, y, w, h};

	SDL_RenderCopy(fw.renderer, img.get_texture(), NULL, &target);
}

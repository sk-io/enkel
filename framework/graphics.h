#pragma once

#include "image.h"

#include <stdint.h>

class Graphics {
public:
	Graphics() {}

	void init();
	void set_color(uint32_t color);
	void clear() const;
	void swap_buffers();
	void fill_rect(float x, float y, float w, float h);
	void draw_img(const Image& img, float x, float y, float w, float h);
private:
	uint32_t color = 0;
};


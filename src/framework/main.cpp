#include <enkel/Lexer.h>
#include <enkel/Parser.h>
#include <enkel/Interpreter.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <iostream>

SDL_Window* window;
SDL_Renderer* renderer;

static char* read_file(const char* path, uint64_t& size) {
	size = 0;
	FILE* file = fopen(path, "rb");
	assert(file);

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (size == 0)
		return nullptr;

	char* buf = (char*) malloc(size + 1);
	fread(buf, size, 1, file);
	buf[size] = 0;

	fclose(file);
	return buf;
}

static void register_funcs(Interpreter& interp) {
	Extern_Func test_func = {"test", 2, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		float x = args[0].as.num;
		float y = args[1].as.num;

		SDL_Rect rect;
		rect.x = (int) x;
		rect.y = (int) y;
		rect.w = 16;
		rect.h = 16;

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderFillRect(renderer, &rect);

		return {};
	}};

	interp.add_external_func(test_func);

	Extern_Func rand_func = {"rand", 0, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		return {Value::from_num(rand() / (float) RAND_MAX)};
	}};

	interp.add_external_func(rand_func);
}

int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO)) {
		assert(false);
		return 1;
	}

	const int width = 800, height = 600;

	window = SDL_CreateWindow("enkel framework",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height,
		0);

	assert(window);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	uint64_t siz;
	char* buf = read_file("input.en", siz);

	auto tokens = Lexer::lex(buf);
	free(buf);

	Parser parser(tokens);
	auto root = parser.parse();

	Interpreter interp;
	register_funcs(interp);

	interp.eval(root.get());

	Value init_func = interp.get_global_scope().find_def("init")->value;
	Value update_func = interp.get_global_scope().find_def("update")->value;
	Value draw_func = interp.get_global_scope().find_def("draw")->value;

	interp.call_function(init_func, {});

	int prev_ticks = SDL_GetTicks();
	int frame_count = 0;
	bool running = true;
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			}
		}

		interp.call_function(update_func, {});

		SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
		SDL_RenderClear(renderer);

		interp.call_function(draw_func, {});

		SDL_RenderPresent(renderer);
		//SDL_Delay(1);

		const int FRAMES = 100;
		if (frame_count++ >= FRAMES) {
			int ticks = SDL_GetTicks() - prev_ticks;

			float ms = ticks / (float) FRAMES;

			std::cout << ms << " ms, " << 1000 / ms << " fps" << std::endl;
			frame_count = 0;
			prev_ticks = SDL_GetTicks();
		}
	}

	SDL_Quit();
}
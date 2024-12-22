#include <enkel/Lexer.h>
#include <enkel/Parser.h>
#include <enkel/Interpreter.h>
#include <enkel/AST_Util.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <stdlib.h>

SDL_Window* window;
SDL_Renderer* renderer;

struct {
	int width = 800, height = 600;
	int scale = 1;
	std::string title = "enkel framework";
} config;

struct {
	uint32_t color = 0xFF00FF;
} draw_state;

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
	interp.add_external_func({"test", 2, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
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
	}});

	interp.add_external_func({"rand", 0, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		return {Value::from_num(rand() / (float) RAND_MAX)};
	}});

	interp.add_external_func({"set_size", 2, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		int width = args[0].as.num;
		int height = args[1].as.num;

		config.width = width;
		config.height = height;

		interp.get_global_scope().set_def("width", Value::from_num(width));
		interp.get_global_scope().set_def("height", Value::from_num(height));

		return {};
	}});

	interp.add_external_func({"set_color", 1, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		const std::string& str = interp.get_string(args[0]);

		if (str[0] != '#')
			assert(false);

		char* end;
		uint32_t i = strtol(str.c_str() + 1, &end, 16);

		if (*end != 0) {
			assert(false);
		}

		int r = i >> 16 & 0xFF;
		int g = i >> 8 & 0xFF;
		int b = i & 0xFF;

		SDL_SetRenderDrawColor(renderer, r, g, b, 255);
		return {};
	}});

	interp.add_external_func({"fill_rect", 4, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		float x = args[0].as.num;
		float y = args[1].as.num;
		float w = args[2].as.num;
		float h = args[3].as.num;

		const SDL_Rect rect = {x, y, w, h};

		SDL_RenderFillRect(renderer, &rect);
		return {};
	}});

	interp.add_external_func({"set_title", 1, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		config.title = interp.get_string(args[0]);
		return {};
	}});
}

static void error(const std::string& msg = "") {
	std::cout << "Error: " << msg.c_str() << std::endl;
	assert(false);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", msg.c_str(), window);
	exit(1);
}

static void init_sdl() {
	if (SDL_Init(SDL_INIT_VIDEO)) {
		error("failed to initialize SDL2");
	}

	window = SDL_CreateWindow(config.title.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		config.width, config.height, 0);
	if (!window) {
		error("failed to create SDL2 window");
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		error("failed to create SDL2 renderer");
	}
}

int main(int argc, char* argv[]) {
	uint64_t siz;
	char* buf = read_file("input.en", siz);

	auto tokens = Lexer::lex(buf);
	free(buf);

	Parser parser(tokens, error);
	auto root = parser.parse();

	//print_ast(root.get());

	Interpreter interp(error);
	register_funcs(interp);

	interp.eval(root.get());

	interp.get_global_scope().set_def("width", Value::from_num(config.width));
	interp.get_global_scope().set_def("height", Value::from_num(config.height));

	Value init_func = interp.get_global_scope().find_def("init")->value;
	Value update_func = interp.get_global_scope().find_def("update")->value;
	Value draw_func = interp.get_global_scope().find_def("draw")->value;

	interp.call_function(init_func, {});

	init_sdl();

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
	return 0;
}
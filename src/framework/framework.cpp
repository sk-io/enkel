

#include <enkel/Lexer.h>
#include <enkel/Parser.h>
#include <enkel/Interpreter.h>
#include <enkel/AST_Util.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <stdlib.h>

#include "framework.h"
#include "graphics.h"

Framework fw{};
Graphics gfx{};

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

static void register_funcs() {
	// --- window ---
	fw.interp.add_external_func({"set_size", 2, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		int width = args[0].as.num;
		int height = args[1].as.num;

		fw.width = width;
		fw.height = height;

		fw.interp.get_global_scope().set_def("width", Value::from_num(width));
		fw.interp.get_global_scope().set_def("height", Value::from_num(height));

		SDL_SetWindowSize(fw.window, width, height);
		return {};
	}});

	fw.interp.add_external_func({"set_title", 1, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		fw.title = fw.interp.get_string(args[0]);
		SDL_SetWindowTitle(fw.window, fw.title.c_str());
		return {};
	}});

	fw.interp.add_external_func({"set_resizable", 1, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		bool resizable = args[0].as._bool;
		SDL_SetWindowResizable(fw.window, (SDL_bool) resizable);
		return {};
	}});

	// --- graphics ---
	fw.interp.add_external_func({"clear", 0, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		gfx.clear();
		return {};
	}});

	fw.interp.add_external_func({"set_color", 1, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		const std::string& str = fw.interp.get_string(args[0]);

		if (str[0] != '#')
			assert(false);

		char* end;
		uint32_t i = strtol(str.c_str() + 1, &end, 16);

		if (*end != 0) {
			assert(false);
		}

		gfx.set_color(i);
		return {};
	}});

	fw.interp.add_external_func({"fill_rect", 4, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		float x = args[0].as.num;
		float y = args[1].as.num;
		float w = args[2].as.num;
		float h = args[3].as.num;

		gfx.fill_rect(x, y, w, h);
		return {};
	}});

	fw.interp.add_external_func({"load_image", 1, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		const std::string& path = interp.get_string(args[0]);
		int id = fw.images.size();
		fw.images.push_back(Image(path));
		return {Value::from_num(id)};
	}});
	
	fw.interp.add_external_func({"draw_image", 3, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		int id = (int) args[0].as.num;
		float x = args[1].as.num;
		float y = args[2].as.num;

		const Image& img = fw.images[id];
		float w = args.size() >= 4 ? args[3].as.num : img.get_width();
		float h = args.size() >= 5 ? args[4].as.num : img.get_height();

		gfx.draw_img(img, x, y, w, h);
		return {};
	}});

	// --- input ---
	fw.interp.add_external_func({"key_down", 1, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		SDL_Keycode code = SDL_GetKeyFromName(interp.get_string(args[0]).c_str());

		if (code == SDLK_UNKNOWN) {
			framework_error("unknown key");
		}

		return {Value::from_bool(is_key_down(code))};
	}});

	// --- utils ---
	fw.interp.add_external_func({"rand", 0, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		return {Value::from_num(rand() / (float) RAND_MAX)};
	}});
}

void framework_error(const std::string& msg) {
	std::cout << "Error: " << msg.c_str() << std::endl;
	assert(false);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", msg.c_str(), fw.window);
	exit(1);
}

bool is_key_down(SDL_Keycode keycode) {
	if (fw.keyboard_state.find(keycode) == fw.keyboard_state.end())
		return false;

	return fw.keyboard_state[keycode];
}

static void init_sdl() {
	if (SDL_Init(SDL_INIT_VIDEO)) {
		framework_error("failed to initialize SDL2");
	}

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		framework_error("failed to initialize SDL2 image");
	}

	fw.window = SDL_CreateWindow(fw.title.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		fw.width, fw.height, SDL_WINDOW_HIDDEN);
	if (!fw.window) {
		framework_error("failed to create SDL2 window");
	}

	gfx.init();
}

static void init() {
	init_sdl();

	uint64_t siz;
	char* buf = read_file("input.en", siz);

	auto tokens = Lexer::lex(buf);
	free(buf);

	Parser parser(tokens);
	parser.set_error_callback(framework_error);
	auto root = parser.parse();

	//print_ast(root.get());

	fw.interp.set_error_callback(framework_error);
	register_funcs();

	fw.interp.eval(root.get());

	fw.interp.get_global_scope().set_def("width", Value::from_num(fw.width));
	fw.interp.get_global_scope().set_def("height", Value::from_num(fw.height));

	fw.interp.get_global_scope().set_def("mouse_x", Value::from_num(0));
	fw.interp.get_global_scope().set_def("mouse_y", Value::from_num(0));

	fw.init_func = fw.interp.get_global_scope().find_def("init")->value;
	fw.update_func = fw.interp.get_global_scope().find_def("update")->value;
	fw.draw_func = fw.interp.get_global_scope().find_def("draw")->value;

	fw.interp.call_function(fw.init_func, {});

	SDL_ShowWindow(fw.window);
}

void run_framework() {
	init();

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
			case SDL_MOUSEMOTION:
				fw.interp.get_global_scope().set_def("mouse_x", Value::from_num(event.motion.x));
				fw.interp.get_global_scope().set_def("mouse_y", Value::from_num(event.motion.y));
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					fw.interp.get_global_scope().set_def("width", Value::from_num(event.window.data1));
					fw.interp.get_global_scope().set_def("height", Value::from_num(event.window.data2));
				}
				break;
			case SDL_KEYDOWN:
				fw.keyboard_state[event.key.keysym.sym] = true;
				break;
			case SDL_KEYUP:
				fw.keyboard_state[event.key.keysym.sym] = false;
				break;
			}
		}
		SDL_GetKeyboardState(NULL);
		fw.interp.call_function(fw.update_func, {});

		fw.interp.call_function(fw.draw_func, {});

		gfx.swap_buffers();

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

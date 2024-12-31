#include "framework.h"
#include "graphics.h"

#include <enkel/lexer.h>
#include <enkel/parser.h>
#include <enkel/interpreter.h>
#include <enkel/ast_util.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <chrono>

Framework fw{};
Graphics gfx{};

static char* read_file(const std::string& path, uint64_t& size) {
	size = 0;
	FILE* file = fopen(path.c_str(), "rb");
	if (!file) {
		framework_error("Failed to open file: " + path);
	}

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
	fw.interp.add_external_func({"key_pressed", 1, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		SDL_Keycode code = SDL_GetKeyFromName(interp.get_string(args[0]).c_str());

		if (code == SDLK_UNKNOWN) {
			framework_error("unknown key");
		}

		return {Value::from_bool(is_key_down(code))};
	}});

	fw.interp.add_external_func({"mouse_pressed", 1, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		const auto& button = interp.get_string(args[0]);

		bool result;
		if (button == "left") {
			result = fw.mouse_left;
		} else if (button == "right") {
			result = fw.mouse_right;
		} else if (button == "middle") {
			result = fw.mouse_middle;
		} else {
			framework_error("Unknown mouse button");
		}

		return {Value::from_bool(result)};
	}});

	// --- utils ---
	fw.interp.add_external_func({"rand", 0, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		return {Value::from_num(rand() / (RAND_MAX + 1.0f))};
	}});

	fw.interp.add_external_func({"set_framerate", 1, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		float fps = args[0].as.num;
		fw.framerate = fps;
		return {};
	}});

	fw.interp.add_external_func({"exit", 0, [&](Interpreter& interp, const std::vector<Value>& args) -> Value {
		fw.running = false;
		return {};
	}});
}

void framework_error(const std::string& msg, const Source_Info* info) {
	std::string final_msg;

	if (info != nullptr) {
		final_msg += "In ";
		final_msg += fw.script_paths[info->file_index];
		final_msg += ", line ";
		final_msg += std::to_string(info->line + 1);
		final_msg += "\n\n";
	}

	final_msg += msg;

	std::cout << final_msg << std::endl;
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", final_msg.c_str(), fw.window);
	assert(false);
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

static std::vector<Token> load_tokens(const std::string& script_path) {
	uint64_t siz;
	char* buf = read_file(script_path, siz);

	int file_index = fw.script_paths.size();

	std::vector<Token> tokens = Lexer::lex(buf);
	free(buf);

	fw.script_paths.push_back(script_path);

	for (auto& token : tokens)
		token.src_info.file_index = file_index;

	return tokens;
}

static bool is_imported(const std::string& script_path) {
	for (const auto& path : fw.script_paths) {
		if (script_path == path)
			return true;
	}
	return false;
}

static void init(const std::string& script_path) {
	init_sdl();
	std::vector<Token> tokens = load_tokens(script_path);

	// scan for import statements
	for (int i = 0; i < tokens.size() - 2; i++) {
		if (tokens[i].type == Token_Type::Keyword_Import &&
			tokens[i + 1].type == Token_Type::String_Literal &&
			tokens[i + 2].type == Token_Type::Semicolon) {
			std::string import_path = tokens[i + 1].str;
			tokens.erase(tokens.begin() + i, tokens.begin() + i + 3);

			if (!is_imported(import_path)) {
				std::vector<Token> imported_tokens = load_tokens(import_path);
				tokens.insert(tokens.begin() + i, imported_tokens.begin(), imported_tokens.end());
			}

			// rescan current token
			i--;
		}
	}

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

	fw.interp.get_global_scope().set_def("delta_time", Value::from_num(0));

	auto try_get_func = [] (const std::string& name) -> Value {
		Definition* def = fw.interp.get_global_scope().find_def(name);
		if (def == nullptr)
			return {};
		return def->value;
	};

	fw.init_func = try_get_func("init");
	fw.update_func = try_get_func("update");
	fw.draw_func = try_get_func("draw");

	if (fw.init_func.type != Value_Type::Null)
		fw.interp.call_function(fw.init_func, {});

	SDL_ShowWindow(fw.window);
}

void run_framework(const std::string& script_path) {
	using namespace std::chrono;

	srand(time(0));

	init(script_path);

	int prev_ticks = SDL_GetTicks();
	int frame_count = 0;

	auto prev_frame_time = high_resolution_clock::now();

	while (fw.running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				fw.running = false;
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
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				//std::cout << std::to_string(event.button.button) << std::endl;
				if (event.button.button == 1)
					fw.mouse_left = event.button.state;
				else if (event.button.button == 2)
					fw.mouse_middle = event.button.state;
				else if (event.button.button == 3)
					fw.mouse_right = event.button.state;
				break;
			}
		}

		auto now = high_resolution_clock::now();
		auto nanos_since_last = duration_cast<nanoseconds>(now - prev_frame_time).count();
		double seconds_since_last = nanos_since_last / 1000000000.0;

		if (fw.framerate <= 0 || seconds_since_last >= 1 / fw.framerate) {
			prev_frame_time = high_resolution_clock::now() - milliseconds(1);

			fw.interp.get_global_scope().set_def("delta_time", Value::from_num(seconds_since_last));

			if (fw.update_func.type != Value_Type::Null)
				fw.interp.call_function(fw.update_func, {});

			if (fw.draw_func.type != Value_Type::Null)
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

		SDL_Delay(1);
	}

	SDL_Quit();
}

#include "framework.h"

#include <enkel/bc_vm.h>
#include <enkel/bc_compiler.h>
#include <enkel/bc_util.h>
#include <enkel/lexer.h>
#include <enkel/parser.h>
#include <enkel/ast_util.h>

#include <SDL2/SDL.h>
#include <iostream>

void testo() {
	// TODO: lookup extern funcs using string/string_id
	// so it wont break if extern_funcs order changes
	// we could even do a link stage before runtime

	// TODO: store runtime data in struct common to both Interpreter and BC_VM
	// things like heap data, strings, etc..
	// and pass that to extern funcs instead
	std::vector<Extern_Func> extern_funcs;
	extern_funcs.push_back({"print", 1, [](const std::vector<Value>& args, void* data_ptr) -> Value {
		std::cout << "[BC] print():  " << args[0].as.num << "\n";
		return {};
	}});

	//auto tokens = Lexer::lex("var x = 5; while (x <= 69) { x += 1; } ");
	auto tokens = Lexer::lex("func test(x, y) { return x - y; } if (1 < 100) print(test(2, 1));");
	Parser parser(tokens);
	auto ast = parser.parse();

	print_ast(ast.get());

	BC_Compiler compiler(extern_funcs);
	auto program = compiler.compile(ast.get());

	std::cout << std::endl;

	print_bc_program(program);

	std::cout << std::endl;

	//BC_Program program;
	//program.code = {
	//	BC_CALL_U8, 0,
	//	BC_EXIT,

	//	BC_ALLOC_FRAME_U8, 3,
	//	BC_PUSH_U8, 5,
	//	BC_PUSH_U8, 8,
	//	BC_ADD,
	//	BC_RET,
	//};
	//program.func_table = {
	//	{3},
	//};

	BC_VM vm;
	vm.run(&program, extern_funcs);

	exit(0);
}

int main(int argc, char* argv[]) {
	testo();
	
	if (argc <= 1) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Please call this executable with a path to the starting script", NULL);
		return 1;
	}

	run_framework(argv[1]);
	return 0;
}

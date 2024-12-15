#include "Interpreter.h"
#include "Scope.h"
#include "Lexer.h"
#include "Parser.h"
#include "AST_Util.h"

#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <memory>

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

int main(int argc, char** argv) {
	std::cout << "enkel script v1.0\n";

	uint64_t size;
	char* buf = read_file("input.en", size);
	//std::cout << buf << std::endl;

	auto tokens = Lexer::lex(buf);
	
	Parser parser(tokens);
	auto program = parser.parse();
	//print_ast(program.get());

	Interpreter interp;

	{
		Extern_Func test;
		test.name = "print";
		test.args.push_back({"doesntmatter"});
		test.callback = [](Interpreter& interp, const std::vector<Value>& args) -> Value {
			std::cout << "print(): " << interp.val_to_str(args[0]) << "\n";
			return Value::null_value();
		};

		interp.add_external_func(test);

		Eval_Result result = interp.eval(program.get());
		//std::cout << result.value.as.num << std::endl;
	}

	interp.get_heap().garbage_collect(interp.get_global_scope());
}
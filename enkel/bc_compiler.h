#pragma once

#include "ast.h"
#include "bc.h"

class BC_Compiler {
public:
	BC_Program compile(AST_Node* node);

private:
	BC_Program program;

	struct BC_Frame {
		std::vector<std::string> vars;
	};

	void compile_node(AST_Node* node, BC_Frame& frame);

	// little endian
	void output_u8(uint8_t byte);
	void output_u16(uint16_t word);
	void output_u32(uint32_t word);

	void write_u8_at(uint8_t word, uint32_t pos = -1);
	void write_u32_at(uint32_t word, uint32_t pos = -1);

	void error(const std::string& msg = "") const;
	int find_var_index(const std::string& name, BC_Frame& frame);
};

#pragma once

#include <vector>

enum {
	BC_EXIT = 0,
	BC_ALLOC_FRAME_U8,
	BC_PUSH_VAR_U8,
	BC_PUSH_U8,
	BC_PUSH_TRUE,
	BC_PUSH_FALSE,
	BC_POP_VAR_U8,
	BC_POP_DISPOSE,
	BC_CALL_U8,
	BC_RET,
	BC_ADD,
	BC_SUB,
	BC_MUL,
	BC_DIV,
	BC_JUMP_U32,
	BC_JUMP_IF_TRUE_U32,
	BC_JUMP_IF_FALSE_U32,
};

struct BC_Func {
	uint32_t entry;
	//bool external = false;
	//std::string name;
};

struct BC_Program {
	std::vector<uint8_t> code;
	std::vector<BC_Func> func_table;
};

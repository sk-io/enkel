#pragma once

#include "extern_func.h"

#include <vector>

enum {
	BC_EXIT = 0,
	BC_ALLOC_FRAME_U8,
	BC_PUSH_VAR_U8,
	BC_PUSH_U8,
	BC_PUSH_F32,
	BC_PUSH_TRUE,
	BC_PUSH_FALSE,
	BC_PUSH_NULL,
	BC_PUSH_FUNC_REF_U32,
	BC_POP_VAR_U8,
	BC_POP_DISPOSE,
	BC_CALL,				// pops func ref value from stack
	BC_CALL_EXTERN_U16,		// doesn't pop
	BC_RET,
	BC_ADD,
	BC_SUB,
	BC_MUL,
	BC_DIV,
	BC_GREATER_THAN,
	BC_LESS_THAN,
	BC_GREATER_THAN_EQUALS,
	BC_LESS_THAN_EQUALS,
	BC_EQUALS,
	BC_NOT_EQUALS,
	BC_JUMP_U32,
	BC_JUMP_IF_TRUE_U32,
	BC_JUMP_IF_FALSE_U32,
};

struct AST_Node;

struct BC_Func {
	uint32_t entry;
	std::string name;
	AST_Node* body_node; // only used during compilation
};

struct BC_Program {
	std::vector<uint8_t> code;
	std::vector<BC_Func> func_table;
	// TODO: store string table
};

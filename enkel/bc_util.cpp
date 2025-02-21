#include "bc_util.h"

#include <stdint.h>
#include <string_view>
#include <assert.h>
#include <iostream>

static std::string_view opcode_to_str(uint8_t opc) {
	switch (opc) {
	case BC_ADD: return "add";
	case BC_EXIT: return "exit";
	case BC_SUB: return "sub";
	case BC_MUL: return "mul";
	case BC_DIV: return "div";
	case BC_POP_DISPOSE: return "pop_dispose";
	case BC_RET: return "ret";
	case BC_ALLOC_FRAME_U8: return "alloc";
	case BC_PUSH_VAR_U8: return "push_var";
	case BC_PUSH_U8: return "push_u8";
	case BC_POP_VAR_U8: return "pop_var";
	case BC_CALL_U8: return "call";
	case BC_PUSH_TRUE: return "push_true";
	case BC_PUSH_FALSE: return "push_false";
	case BC_JUMP_U32: return "jump";
	case BC_JUMP_IF_TRUE_U32: return "jump_if_true";
	case BC_JUMP_IF_FALSE_U32: return "jump_if_false";
	}
	assert(false);
}

static int get_inst_len(uint8_t opc) {
	switch (opc) {
	case BC_EXIT:
	case BC_ADD:
	case BC_SUB:
	case BC_MUL:
	case BC_DIV:
	case BC_POP_DISPOSE:
	case BC_RET:
	case BC_PUSH_TRUE:
	case BC_PUSH_FALSE:
		return 1;
	case BC_ALLOC_FRAME_U8:
	case BC_PUSH_VAR_U8:
	case BC_PUSH_U8:
	case BC_POP_VAR_U8:
	case BC_CALL_U8:
		return 2;
	case BC_JUMP_U32:
	case BC_JUMP_IF_TRUE_U32:
	case BC_JUMP_IF_FALSE_U32:
		return 5;
	}

	assert(false);
}

void print_bc_program(const BC_Program& program) {
	uint32_t pos = 0;

	while (pos < program.code.size()) {
		uint8_t opc = program.code[pos];
		
		std::cout << "    " << opcode_to_str(opc) << " ";

		int inst_len = get_inst_len(opc);

		for (int i = 1; i < inst_len; i++)
			std::cout << (int) program.code[pos + i] << " ";

		std::cout << "\n";

		pos += inst_len;
	}
}

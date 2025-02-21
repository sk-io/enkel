#include "bc_vm.h"

#include <assert.h>

void BC_VM::run(const BC_Program* program) {
	BC_VM::program = program;

	pos = 0;

	while (pos < program->code.size()) {
		uint8_t op = eat_u8();

		switch (op) {
		case BC_EXIT:
			return;
		case BC_ALLOC_FRAME_U8: {
			uint32_t num_vars = eat_u8();
			frame_stack.push_back({(uint32_t)var_stack.size(), num_vars});
			for (int i = 0; i < num_vars; i++)
				var_stack.push_back({});
			break;
		}
		case BC_PUSH_U8:
			op_stack.push_back(Value::from_num(eat_u8()));
			break;
		case BC_PUSH_TRUE:
			op_stack.push_back(Value::from_bool(true));
			break;
		case BC_PUSH_FALSE:
			op_stack.push_back(Value::from_bool(false));
			break;
		case BC_PUSH_VAR_U8: {
			int var_index = eat_u8();
			const BC_Frame& cur_frame = frame_stack.back();

			op_stack.push_back(var_stack[cur_frame.start + var_index]);
			break;
		}
		case BC_POP_DISPOSE:
			op_stack.pop_back();
			break;
		case BC_POP_VAR_U8: {
			int var_index = eat_u8();
			const BC_Frame& cur_frame = frame_stack.back();

			var_stack[cur_frame.start + var_index] = op_stack.back();
			op_stack.pop_back();
			break;
		}
		case BC_CALL_U8: {
			const BC_Func& func = program->func_table[eat_u8()];

			call_stack.push_back(pos);
			pos = func.entry;
			break;
		}
		case BC_RET: {
			pos = call_stack.back();
			call_stack.pop_back();
			const BC_Frame& cur_frame = frame_stack.back();
			for (int i = 0; i < cur_frame.num_vars; i++)
				var_stack.pop_back();
			frame_stack.pop_back();
			break;
		}
		case BC_ADD:
		case BC_SUB:
		case BC_MUL:
		case BC_DIV:
		{
			float b = op_stack.back().as.num;
			op_stack.pop_back();
			float a = op_stack.back().as.num;
			op_stack.pop_back();

			float result;
			switch (op) {
			case BC_ADD: result = a + b; break;
			case BC_SUB: result = a - b; break;
			case BC_MUL: result = a * b; break;
			case BC_DIV: result = a / b; break;
			}

			op_stack.push_back(Value::from_num(result));
			break;
		}
		case BC_JUMP_U32:
			pos = eat_u32();
			break;
		case BC_JUMP_IF_TRUE_U32:
		case BC_JUMP_IF_FALSE_U32: {
			uint32_t dest = eat_u32();
			bool b = op_stack.back().as._bool;
			op_stack.pop_back();

			if ((op != BC_JUMP_IF_TRUE_U32) != b)
				pos = dest;
			break;
		}
		default:
			assert(false);
		}
	}
}

uint8_t BC_VM::eat_u8() {
	return program->code[pos++];
}

uint16_t BC_VM::eat_u16() {
	uint16_t result = program->code[pos++];
	result |= program->code[pos++] << 8;
	return result;
}

uint32_t BC_VM::eat_u32() {
	uint32_t result = program->code[pos++];
	result |= program->code[pos++] << 8;
	result |= program->code[pos++] << 16;
	result |= program->code[pos++] << 24;
	return result;
}

float BC_VM::eat_float() {
	return 0.0f;
}

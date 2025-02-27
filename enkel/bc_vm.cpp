#include "bc_vm.h"

#include <assert.h>
#include <algorithm>

void BC_VM::run(const BC_Program* program, const std::vector<Extern_Func>& extern_funcs) {
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
		case BC_PUSH_F32:
			op_stack.push_back(Value::from_num(eat_f32()));
			break;
		case BC_PUSH_TRUE:
			op_stack.push_back(Value::from_bool(true));
			break;
		case BC_PUSH_FALSE:
			op_stack.push_back(Value::from_bool(false));
			break;
		case BC_PUSH_NULL:
			op_stack.push_back(Value::null_value());
			break;
		case BC_PUSH_VAR_U8: {
			int var_index = eat_u8();
			const BC_Frame& cur_frame = frame_stack.back();

			op_stack.push_back(var_stack[cur_frame.start + var_index]);
			break;
		}
		case BC_PUSH_FUNC_REF_U32: {
			uint32_t func_index = eat_u32();

			Value val{};
			val.type = Value_Type::BC_Func_Ref;
			val.as.i = func_index;

			op_stack.push_back(val);
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
		case BC_CALL: {
			Value func_val = op_stack.back();
			op_stack.pop_back();

			if (func_val.type != Value_Type::BC_Func_Ref) {
				assert(false);
			}

			const BC_Func& func = program->func_table[func_val.as.i];

			call_stack.push_back(pos);
			pos = func.entry;
			break;
		}
		case BC_CALL_EXTERN_U16: {
			uint16_t extern_id = eat_u16();

			const Extern_Func& func = extern_funcs[extern_id];
			std::vector<Value> args;

			// TODO: optional args
			for (int i = 0; i < func.min_args; i++) {
				args.push_back(op_stack.back());
				op_stack.pop_back();
			}

			std::reverse(args.begin(), args.end());

			Value ret = func.callback(args, this);
			op_stack.push_back(ret);
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
		case BC_GREATER_THAN:
		case BC_LESS_THAN:
		case BC_GREATER_THAN_EQUALS:
		case BC_LESS_THAN_EQUALS:
		{
			float b = op_stack.back().as.num;
			op_stack.pop_back();
			float a = op_stack.back().as.num;
			op_stack.pop_back();

			Value result;
			switch (op) {
			case BC_ADD: result = Value::from_num(a + b); break;
			case BC_SUB: result = Value::from_num(a - b); break;
			case BC_MUL: result = Value::from_num(a * b); break;
			case BC_DIV: result = Value::from_num(a / b); break;
			case BC_GREATER_THAN: result = Value::from_bool(a > b); break;
			case BC_LESS_THAN: result = Value::from_bool(a < b); break;
			case BC_GREATER_THAN_EQUALS: result = Value::from_bool(a >= b); break;
			case BC_LESS_THAN_EQUALS: result = Value::from_bool(a <= b); break;
			}

			op_stack.push_back(result);
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

float BC_VM::eat_f32() {
	uint32_t bin = eat_u32();

	float num;
	memcpy(&num, &bin, 4);

	return num;
}

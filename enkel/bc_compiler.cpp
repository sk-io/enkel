#include "bc_compiler.h"

#include <iostream>
#include <algorithm>

BC_Program BC_Compiler::compile(AST_Node* node) {
	program = {};

	BC_Frame global_frame;
	compile_node(node, global_frame);

	return program;
}

void BC_Compiler::compile_node(AST_Node* node, BC_Frame& frame) {
	switch (node->type) {
	case AST_Node_Type::Literal: {
		AST_Literal* sub = (AST_Literal*) node;
		switch (sub->val.type) {
		case Value_Type::Num:
			// TODO: implement this properly
			output_u8(BC_PUSH_U8);
			output_u8((uint8_t) sub->val.as.num);
			break;
		case Value_Type::Bool:
			output_u8(sub->val.as._bool ? BC_PUSH_TRUE : BC_PUSH_FALSE);
			break;
		default:
			error("unhandled value type");
		}

		return;
	}
	case AST_Node_Type::Bin_Op: {
		AST_Bin_Op* sub = (AST_Bin_Op*) node;

		if (sub->op == Bin_Op::Assign) {
			compile_node(sub->right.get(), frame);

			if (sub->left->type != AST_Node_Type::Var) {
				error("not implemented yet, sorry.");
			}

			int var_index = find_var_index(((AST_Var*) sub->left.get())->name, frame);

			output_u8(BC_POP_VAR_U8);
			output_u8(var_index);
			
			return;
		}
		
		compile_node(sub->left.get(), frame);
		compile_node(sub->right.get(), frame);

		switch (sub->op) {
		case Bin_Op::Add:
			output_u8(BC_ADD);
			break;
		case Bin_Op::Sub:
			output_u8(BC_SUB);
			break;
		case Bin_Op::Mul:
			output_u8(BC_MUL);
			break;
		case Bin_Op::Div:
			output_u8(BC_DIV);
			break;
		default:
			error("unhandled binary op type");
		}
		
		return;
	}
	case AST_Node_Type::Block: {
		AST_Block* sub = (AST_Block*) node;

		uint32_t num_vars_backpatch;
		if (sub->is_global_scope) {
			output_u8(BC_ALLOC_FRAME_U8);
			num_vars_backpatch = program.code.size();
			output_u8((uint8_t) -1);
		}

		for (auto& statement : sub->statements) {
			compile_node(statement.get(), frame);
		}

		if (sub->is_global_scope) {
			write_u8_at(frame.vars.size(), num_vars_backpatch);
		}

		return;
	}
	case AST_Node_Type::Func_Decl: {
		AST_Func_Decl* sub = (AST_Func_Decl*) node;
		break;
	}
	case AST_Node_Type::Var_Decl: {
		AST_Var_Decl* sub = (AST_Var_Decl*) node;

		int index = frame.vars.size();
		frame.vars.push_back(sub->name);

		if (sub->init != nullptr) {
			compile_node(sub->init.get(), frame);
			
			output_u8(BC_POP_VAR_U8);
			output_u8(index);
			return;
		}
		
		return;
	}
	case AST_Node_Type::Var: {
		AST_Var* sub = (AST_Var*) node;
		int index = find_var_index(sub->name, frame);

		output_u8(BC_PUSH_VAR_U8);
		output_u8(index);
		return;
	}
	case AST_Node_Type::If: {
		AST_If* sub = (AST_If*) node;

		if (sub->else_body != nullptr) {
			error("not implemented yet");
		}

		compile_node(sub->condition.get(), frame);
		output_u8(BC_JUMP_IF_FALSE_U32);
		uint32_t skip_patch_addr = program.code.size();
		output_u32((uint32_t) -1);

		compile_node(sub->if_body.get(), frame);

		// backpatch skip label addr
		write_u32_at(program.code.size(), skip_patch_addr);
		return;
	}
	case AST_Node_Type::While: {
		AST_While* sub = (AST_While*) node;

		// loop:
		uint32_t loop_addr = program.code.size();

		compile_node(sub->condition.get(), frame);
		output_u8(BC_JUMP_IF_FALSE_U32);
		uint32_t loop_patch_addr = program.code.size();
		output_u32((uint32_t) -1);

		// body
		compile_node(sub->body.get(), frame);
		output_u8(BC_JUMP_U32);
		output_u32(loop_addr);

		// exit:
		uint32_t exit_addr = program.code.size();
		write_u32_at(exit_addr, loop_patch_addr);
		return;
	}
	default:
		error("unhandled node type");
	}

	error("yeah");
}

void BC_Compiler::output_u8(uint8_t byte) {
	program.code.push_back(byte);
}

void BC_Compiler::output_u16(uint16_t word) {
	program.code.push_back(word & 0xFF);
	program.code.push_back(word >> 8);
}

void BC_Compiler::output_u32(uint32_t word) {
	program.code.push_back(word & 0xFF);
	program.code.push_back(word >> 8 & 0xFF);
	program.code.push_back(word >> 16 & 0xFF);
	program.code.push_back(word >> 24 & 0xFF);
}

void BC_Compiler::write_u8_at(uint8_t word, uint32_t pos) {
	program.code[pos] = word;
}

void BC_Compiler::write_u32_at(uint32_t word, uint32_t pos) {
	program.code[pos] = word & 0xFF;
	program.code[pos + 1] = word >> 8 & 0xFF;
	program.code[pos + 2] = word >> 16 & 0xFF;
	program.code[pos + 3] = word >> 24 & 0xFF;
}

void BC_Compiler::error(const std::string& msg) const {
	std::cout << "Bytecode compiler error: " << msg << "\n";
	assert(false);
	exit(1);
}

int BC_Compiler::find_var_index(const std::string& name, BC_Frame& frame) {
	auto result = std::find(frame.vars.begin(), frame.vars.end(), name);

	if (result == frame.vars.end()) {
		error("variable not found in current frame");
	}

	return result - frame.vars.begin();
}

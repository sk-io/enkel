#include "AST_Util.h"

#include <iostream>
#include <assert.h>

void print_ast(AST_Node* node, int depth) {
	for (int i = 0; i < depth; i++)
		std::cout << " ";

	switch (node->type) {
	case AST_Node_Type::Literal: {
		AST_Literal* sub = (AST_Literal*) node;
		std::cout << "AST_Literal: " << sub->val.as.num << "\n";
		break;
	}
	case AST_Node_Type::String_Literal: {
		AST_String_Literal* sub = (AST_String_Literal*) node;
		std::cout << "AST_String_Literal: \"" << sub->str << "\"\n";
		break;
	}
	case AST_Node_Type::Bin_Op: {
		AST_Bin_Op* sub = (AST_Bin_Op*) node;
		std::cout << "AST_Bin_Op: " << "\n";

		print_ast(sub->left.get(), depth + 1);
		print_ast(sub->right.get(), depth + 1);
		break;
	}
	case AST_Node_Type::Block: {
		AST_Block* sub = (AST_Block*) node;
		std::cout << "AST_Block: " << "\n";

		for (auto& statement : sub->statements) {
			print_ast(statement.get(), depth + 1);
		}
		break;
	}
	case AST_Node_Type::Var_Decl: {
		AST_Var_Decl* sub = (AST_Var_Decl*) node;
		std::cout << "AST_Var_Decl: " << sub->name << "\n";

		if (sub->init != nullptr)
			print_ast(sub->init.get(), depth + 1);

		break;
	}
	case AST_Node_Type::Var: {
		AST_Var* sub = (AST_Var*) node;
		std::cout << "AST_Var: " << sub->name << "\n";
		break;
	}
	case AST_Node_Type::Func_Decl: {
		AST_Func_Decl* sub = (AST_Func_Decl*) node;
		std::cout << "AST_Func_Decl: " << sub->name << "\n";

		print_ast(sub->body.get(), depth + 1);
		break;
	}
	case AST_Node_Type::Func_Call: {
		AST_Func_Call* sub = (AST_Func_Call*) node;
		std::cout << "AST_Func_Call\n";
		print_ast(sub->expr.get(), depth + 1);
		break;
	}
	case AST_Node_Type::If: {
		AST_If* sub = (AST_If*) node;
		std::cout << "AST_If\n";

		print_ast(sub->condition.get(), depth + 1);
		print_ast(sub->if_body.get(), depth + 1);
		if (sub->else_body != nullptr)
			print_ast(sub->else_body.get(), depth + 1);
		break;
	}
	case AST_Node_Type::While: {
		AST_While* sub = (AST_While*) node;
		std::cout << "AST_While\n";

		print_ast(sub->condition.get(), depth + 1);
		print_ast(sub->body.get(), depth + 1);
		break;
	}
	case AST_Node_Type::For: {
		AST_For* sub = (AST_For*) node;
		std::cout << "AST_For\n";
		// TODO: var name
		print_ast(sub->expr.get(), depth + 1);
		print_ast(sub->body.get(), depth + 1);
		break;
	}
	case AST_Node_Type::Return: {
		AST_Return* sub = (AST_Return*) node;
		std::cout << "AST_Return\n";

		print_ast(sub->expr.get(), depth + 1);
		break;
	}
	case AST_Node_Type::Array_Init: {
		AST_Array_Init* sub = (AST_Array_Init*) node;
		std::cout << "AST_Array_Init\n";

		//print_ast(sub->expr.get(), depth + 1);
		break;
	}
	case AST_Node_Type::Subscript: {
		AST_Subscript* sub = (AST_Subscript*) node;
		std::cout << "AST_Subscript\n";

		print_ast(sub->expr.get(), depth + 1);
		print_ast(sub->subscript.get(), depth + 1);
		break;
	}
	case AST_Node_Type::Class_Decl: {
		AST_Class_Decl* sub = (AST_Class_Decl*) node;
		std::cout << "AST_Class_Decl: " << sub->name << "\n";

		for (auto& member : sub->members) {
			print_ast(member.get(), depth + 1);
		}
		break;
	}
	case AST_Node_Type::This: {
		std::cout << "This\n";
		break;
	}
	case AST_Node_Type::New: {
		AST_New* sub = (AST_New*) node;
		std::cout << "New: " << sub->name << "\n";

		// TODO: args
		break;
	}
	default:
		assert(false);
	}
}

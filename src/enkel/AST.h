#pragma once

#include "Value.h"
#include "Definition.h"
#include "Operators.h"

#include <vector>
#include <memory>
#include <string>

enum class AST_Node_Type {
	Literal,
	String_Literal,
	Unary_Op,
	Bin_Op,
	Block,
	Anon_Block,
	Var_Decl,
	Var,
	Func_Decl,
	Return,
	Func_Call,
	If,
	While,
	For,
	Break,
	Continue,
	Array_Init,
	Subscript,
	Class_Decl,
	This,
	New,
	Null,
};

struct AST_Node {
	AST_Node_Type type;

	AST_Node(AST_Node_Type _type) :
		type(_type) {}
};

struct AST_Literal : public AST_Node {
	Value val;

	AST_Literal(Value _val) :
		AST_Node(AST_Node_Type::Literal), val(_val) {}
};

struct AST_String_Literal : public AST_Node {
	std::string str;

	AST_String_Literal(const std::string& _str) :
		AST_Node(AST_Node_Type::String_Literal), str(_str) {}
};

struct AST_Unary_Op : public AST_Node {
	std::unique_ptr<AST_Node> expr;
	Unary_Op op;

	AST_Unary_Op(std::unique_ptr<AST_Node> _expr, Unary_Op _op) :
		AST_Node(AST_Node_Type::Unary_Op), expr(std::move(_expr)), op(_op) {}
};

struct AST_Bin_Op : public AST_Node {
	std::unique_ptr<AST_Node> left;
	std::unique_ptr<AST_Node> right;
	Bin_Op op;

	AST_Bin_Op(std::unique_ptr<AST_Node> _left, std::unique_ptr<AST_Node> _right, Bin_Op _op) :
		AST_Node(AST_Node_Type::Bin_Op), left(std::move(_left)), right(std::move(_right)), op(_op) {}
};

struct AST_Block : public AST_Node {
	std::vector<std::unique_ptr<AST_Node>> statements;

	AST_Block() :
		AST_Node(AST_Node_Type::Block) {}
};

struct AST_Var_Decl : public AST_Node {
	std::string name;
	std::unique_ptr<AST_Node> init;

	AST_Var_Decl(const std::string& _name, std::unique_ptr<AST_Node> _init) :
		AST_Node(AST_Node_Type::Var_Decl), name(_name), init(std::move(_init)) {}
};

// TODO: rename to identifier
struct AST_Var : public AST_Node {
	std::string name;

	AST_Var(const std::string& _name) :
		AST_Node(AST_Node_Type::Var), name(_name) {}
};

struct AST_Func_Decl : public AST_Node {
	std::string name;
	std::unique_ptr<AST_Node> body;
	std::vector<Definition> args;
	bool is_global = false;
	//std::string class_name; // TODO: uhhh

	AST_Func_Decl(const std::string& _name, std::unique_ptr<AST_Node> _body, bool _is_global) :
		AST_Node(AST_Node_Type::Func_Decl), name(_name), body(std::move(_body)), is_global(_is_global) {}
};

struct AST_Return : public AST_Node {
	std::unique_ptr<AST_Node> expr;

	AST_Return(std::unique_ptr<AST_Node> _expr)
		: AST_Node(AST_Node_Type::Return), expr(std::move(_expr)) {}
};

struct AST_Func_Call : public AST_Node {
	std::unique_ptr<AST_Node> expr;
	std::vector<std::unique_ptr<AST_Node>> args;

	AST_Func_Call(std::unique_ptr<AST_Node> _expr) :
		AST_Node(AST_Node_Type::Func_Call), expr(std::move(_expr)) {}
};

struct AST_If : public AST_Node {
	std::unique_ptr<AST_Node> condition;
	std::unique_ptr<AST_Node> if_body;
	std::unique_ptr<AST_Node> else_body;

	AST_If(std::unique_ptr<AST_Node> _condition, std::unique_ptr<AST_Node> _if_body, std::unique_ptr<AST_Node> _else_body) :
		AST_Node(AST_Node_Type::If), condition(std::move(_condition)), if_body(std::move(_if_body)), else_body(std::move(_else_body)) {}
};

struct AST_While : public AST_Node {
	std::unique_ptr<AST_Node> condition;
	std::unique_ptr<AST_Node> body;

	AST_While(std::unique_ptr<AST_Node> _condition, std::unique_ptr<AST_Node> _body) :
		AST_Node(AST_Node_Type::While), condition(std::move(_condition)), body(std::move(_body)) {}
};

struct AST_For : public AST_Node {
	std::string var_name;
	std::unique_ptr<AST_Node> expr;
	std::unique_ptr<AST_Node> body;

	AST_For(const std::string& _var_name, std::unique_ptr<AST_Node> _expr, std::unique_ptr<AST_Node> _body) :
		AST_Node(AST_Node_Type::For), var_name(_var_name), expr(std::move(_expr)), body(std::move(_body)) {}
};

struct AST_Array_Init : public AST_Node {
	std::vector<std::unique_ptr<AST_Node>> items;

	AST_Array_Init() : AST_Node(AST_Node_Type::Array_Init) {}
};

struct AST_Subscript : public AST_Node {
	std::unique_ptr<AST_Node> expr;
	std::unique_ptr<AST_Node> subscript;

	AST_Subscript(std::unique_ptr<AST_Node> _expr, std::unique_ptr<AST_Node> _subscript) :
		AST_Node(AST_Node_Type::Subscript), expr(std::move(_expr)), subscript(std::move(_subscript)) {}
};

struct AST_Class_Decl : public AST_Node {
	std::string name, parent;
	std::vector<std::unique_ptr<AST_Node>> members;

	AST_Class_Decl(const std::string& _name, const std::string& _parent) :
		AST_Node(AST_Node_Type::Class_Decl), name(_name), parent(_parent) {}
};

struct AST_Implied : public AST_Node {
	AST_Implied(AST_Node_Type _type) :
		AST_Node(_type) {}
};

struct AST_New : public AST_Node {
	std::string name;
	std::vector<std::unique_ptr<AST_Node>> args;

	AST_New(const std::string& _name) :
		AST_Node(AST_Node_Type::New), name(_name) {}
};

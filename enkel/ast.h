#pragma once

#include "value.h"
#include "definition.h"
#include "operators.h"
#include "source_info.h"

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
	Multi_Var_Decl,
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
	Import,
};

struct AST_Node {
	AST_Node_Type type;
	Source_Info src_info;

	AST_Node(AST_Node_Type _type, Source_Info _src_info) :
		type(_type), src_info(_src_info) {}
};

struct AST_Literal : public AST_Node {
	Value val;

	AST_Literal(Source_Info _src_info, Value _val) :
		AST_Node(AST_Node_Type::Literal, _src_info), val(_val) {}
};

struct AST_String_Literal : public AST_Node {
	std::string str;

	AST_String_Literal(Source_Info _src_info, const std::string& _str) :
		AST_Node(AST_Node_Type::String_Literal, _src_info), str(_str) {}
};

struct AST_Unary_Op : public AST_Node {
	std::unique_ptr<AST_Node> expr;
	Unary_Op op;

	AST_Unary_Op(Source_Info _src_info, std::unique_ptr<AST_Node> _expr, Unary_Op _op) :
		AST_Node(AST_Node_Type::Unary_Op, _src_info), expr(std::move(_expr)), op(_op) {}
};

struct AST_Bin_Op : public AST_Node {
	std::unique_ptr<AST_Node> left;
	std::unique_ptr<AST_Node> right;
	Bin_Op op;

	AST_Bin_Op(Source_Info _src_info, std::unique_ptr<AST_Node> _left, std::unique_ptr<AST_Node> _right, Bin_Op _op) :
		AST_Node(AST_Node_Type::Bin_Op, _src_info), left(std::move(_left)), right(std::move(_right)), op(_op) {}
};

struct AST_Block : public AST_Node {
	std::vector<std::unique_ptr<AST_Node>> statements;

	AST_Block(Source_Info _src_info) :
		AST_Node(AST_Node_Type::Block, _src_info) {}
};

struct AST_Var_Decl : public AST_Node {
	std::string name;
	std::unique_ptr<AST_Node> init;
	bool is_const;

	AST_Var_Decl(Source_Info _src_info, const std::string& _name, std::unique_ptr<AST_Node> _init, bool _is_const) :
		AST_Node(AST_Node_Type::Var_Decl, _src_info), name(_name), init(std::move(_init)), is_const(_is_const) {}
};

struct AST_Multi_Var_Decl : public AST_Node {
	std::vector<std::unique_ptr<AST_Node>> decls;

	AST_Multi_Var_Decl(Source_Info _src_info) :
		AST_Node(AST_Node_Type::Multi_Var_Decl, _src_info) {}
};

// TODO: rename to identifier
struct AST_Var : public AST_Node {
	std::string name;

	AST_Var(Source_Info _src_info, const std::string& _name) :
		AST_Node(AST_Node_Type::Var, _src_info), name(_name) {}
};

struct AST_Func_Decl : public AST_Node {
	std::string name;
	std::unique_ptr<AST_Node> body;
	std::vector<Definition> args;
	bool is_global = false;
	//std::string class_name; // TODO: uhhh

	AST_Func_Decl(Source_Info _src_info, const std::string& _name, std::unique_ptr<AST_Node> _body, bool _is_global) :
		AST_Node(AST_Node_Type::Func_Decl, _src_info), name(_name), body(std::move(_body)), is_global(_is_global) {}
};

struct AST_Return : public AST_Node {
	std::unique_ptr<AST_Node> expr;

	AST_Return(Source_Info _src_info, std::unique_ptr<AST_Node> _expr)
		: AST_Node(AST_Node_Type::Return, _src_info), expr(std::move(_expr)) {}
};

struct AST_Func_Call : public AST_Node {
	std::unique_ptr<AST_Node> expr;
	std::vector<std::unique_ptr<AST_Node>> args;

	AST_Func_Call(Source_Info _src_info, std::unique_ptr<AST_Node> _expr) :
		AST_Node(AST_Node_Type::Func_Call, _src_info), expr(std::move(_expr)) {}
};

struct AST_If : public AST_Node {
	std::unique_ptr<AST_Node> condition;
	std::unique_ptr<AST_Node> if_body;
	std::unique_ptr<AST_Node> else_body;

	AST_If(Source_Info _src_info, std::unique_ptr<AST_Node> _condition, std::unique_ptr<AST_Node> _if_body, std::unique_ptr<AST_Node> _else_body) :
		AST_Node(AST_Node_Type::If, _src_info), condition(std::move(_condition)), if_body(std::move(_if_body)), else_body(std::move(_else_body)) {}
};

struct AST_While : public AST_Node {
	std::unique_ptr<AST_Node> condition;
	std::unique_ptr<AST_Node> body;

	AST_While(Source_Info _src_info, std::unique_ptr<AST_Node> _condition, std::unique_ptr<AST_Node> _body) :
		AST_Node(AST_Node_Type::While, _src_info), condition(std::move(_condition)), body(std::move(_body)) {}
};

struct AST_For : public AST_Node {
	std::string var_name;
	std::unique_ptr<AST_Node> expr;
	std::unique_ptr<AST_Node> body;

	AST_For(Source_Info _src_info, const std::string& _var_name, std::unique_ptr<AST_Node> _expr, std::unique_ptr<AST_Node> _body) :
		AST_Node(AST_Node_Type::For, _src_info), var_name(_var_name), expr(std::move(_expr)), body(std::move(_body)) {}
};

struct AST_Array_Init : public AST_Node {
	std::vector<std::unique_ptr<AST_Node>> items;

	AST_Array_Init(Source_Info _src_info) :
		AST_Node(AST_Node_Type::Array_Init, _src_info) {}
};

struct AST_Subscript : public AST_Node {
	std::unique_ptr<AST_Node> expr;
	std::unique_ptr<AST_Node> subscript;

	AST_Subscript(Source_Info _src_info, std::unique_ptr<AST_Node> _expr, std::unique_ptr<AST_Node> _subscript) :
		AST_Node(AST_Node_Type::Subscript, _src_info), expr(std::move(_expr)), subscript(std::move(_subscript)) {}
};

struct AST_Class_Decl : public AST_Node {
	std::string name, parent;
	std::vector<std::unique_ptr<AST_Node>> members;

	AST_Class_Decl(Source_Info _src_info, const std::string& _name, const std::string& _parent) :
		AST_Node(AST_Node_Type::Class_Decl, _src_info), name(_name), parent(_parent) {}
};

struct AST_Implied : public AST_Node {
	AST_Implied(Source_Info _src_info, AST_Node_Type _type) :
		AST_Node(_type, _src_info) {}
};

struct AST_New : public AST_Node {
	std::string name;
	std::vector<std::unique_ptr<AST_Node>> args;

	AST_New(Source_Info _src_info, const std::string& _name) :
		AST_Node(AST_Node_Type::New, _src_info), name(_name) {}
};

struct AST_Import : public AST_Node {
	std::string path;

	AST_Import(Source_Info _src_info, const std::string& _path) :
		AST_Node(AST_Node_Type::Import, _src_info), path(_path) {}
};

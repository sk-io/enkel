#pragma once

#include "ast.h"
#include "token.h"

#include <memory>
#include <vector>
#include <functional>

class Parser {
public:
	using Error_Callback_Func = std::function<void(const std::string& msg, const Source_Info* info)>;

	Parser(std::vector<Token>&&) = delete;
	Parser(const std::vector<Token>& _tokens) : tokens(_tokens) {}

	std::unique_ptr<AST_Node> parse();
	std::unique_ptr<AST_Node> parse_block();
	std::unique_ptr<AST_Node> parse_statement();
	std::unique_ptr<AST_Node> parse_expression();
	std::unique_ptr<AST_Node> parse_infix(int min_prec);
	std::unique_ptr<AST_Node> parse_prefix();
	std::unique_ptr<AST_Node> parse_postfix();
	std::unique_ptr<AST_Node> parse_primary();

	std::unique_ptr<AST_Node> parse_var_decl();
	std::unique_ptr<AST_Node> parse_func_decl(bool is_global);
	std::unique_ptr<AST_Node> parse_class_decl();

	void set_error_callback(Error_Callback_Func _func) { error_callback = _func; }

private:
	const Token& peek(int offset = 0);
	const Token& eat(Token_Type expected = Token_Type::Any);

	void error(const std::string& msg = "") const;

	int pos = 0;
	const std::vector<Token>& tokens;
	Error_Callback_Func error_callback;
};
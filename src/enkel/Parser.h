#pragma once

#include "AST.h"
#include "Token.h"

#include <memory>
#include <vector>

class Parser {
public:
	Parser(std::vector<Token>&&) = delete;
	Parser(const std::vector<Token>& _tokens) : tokens(_tokens) {}

	std::unique_ptr<AST_Node> parse();
	std::unique_ptr<AST_Node> parse_block();
	std::unique_ptr<AST_Node> parse_statement();
	std::unique_ptr<AST_Node> parse_expression();
	std::unique_ptr<AST_Node> parse_infix(int min_prec);
	std::unique_ptr<AST_Node> parse_postfix();
	std::unique_ptr<AST_Node> parse_primary();

	std::unique_ptr<AST_Node> parse_var_decl();
	std::unique_ptr<AST_Node> parse_func_decl();
	std::unique_ptr<AST_Node> parse_class_decl();

private:
	const Token& peek(int offset = 0);
	const Token& eat(Token_Type expected);

	void error(const std::string& msg = "") const;

	int pos = 0;
	const std::vector<Token>& tokens;
};
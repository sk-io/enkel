#include "lexer.h"

static Token consume_identifier(const std::string& input, int& pos) {
	int start = pos;

	while (pos + 1 < input.length()) {
		char next = input[pos + 1];

		if (!isalnum(next) && next != '_')
			break;

		pos++;
	}

	std::string str = input.substr(start, pos - start + 1);

	Token_Type type = Token_Type::Identifier;
	Value val;

	if (str == "if")
		type = Token_Type::Keyword_If;
	else if (str == "func")
		type = Token_Type::Keyword_Func;
	else if (str == "pass")
		type = Token_Type::Keyword_Pass;
	else if (str == "while")
		type = Token_Type::Keyword_While;
	else if (str == "for")
		type = Token_Type::Keyword_For;
	else if (str == "in")
		type = Token_Type::Keyword_In;
	else if (str == "continue")
		type = Token_Type::Keyword_Continue;
	else if (str == "break")
		type = Token_Type::Keyword_Break;
	else if (str == "return")
		type = Token_Type::Keyword_Return;
	else if (str == "var")
		type = Token_Type::Keyword_Var;
	else if (str == "class")
		type = Token_Type::Keyword_Class;
	else if (str == "constructor")
		type = Token_Type::Keyword_Constructor;
	else if (str == "this")
		type = Token_Type::Keyword_This;
	else if (str == "new")
		type = Token_Type::Keyword_New;
	else if (str == "extends")
		type = Token_Type::Keyword_Extends;
	else if (str == "is")
		type = Token_Type::Keyword_Is;
	else if (str == "null")
		type = Token_Type::Keyword_Null;
	else if (str == "else")
		type = Token_Type::Keyword_Else;
	else if (str == "not")
		type = Token_Type::Keyword_Not;
	else if (str == "and")
		type = Token_Type::Keyword_And;
	else if (str == "or")
		type = Token_Type::Keyword_Or;
	else if (str == "import")
		type = Token_Type::Keyword_Import;

	if (str == "true") {
		type = Token_Type::Boolean_Literal;
		val = Value::from_bool(true);
	} else if (str == "false") {
		type = Token_Type::Boolean_Literal;
		val = Value::from_bool(false);
	}

	return Token{
		type,
		val,
		str
	};
}

static Token consume_number(const std::string& input, int& pos) {
	int start = pos;

	while (pos + 1 < input.length()) {
		char next = input[pos + 1];

		if (!isdigit(next) && next != '.')
			break;

		pos++;
	}

	auto str = input.substr(start, pos - start + 1);
	Value val = Value::from_num(std::stof(str));

	return Token{
		Token_Type::Number_Literal,
		val,
		str
	};
}

static Token consume_indentation(const std::string& input, int& pos) {
	int start = pos;

	while (pos + 1 < input.length()) {
		char next = input[pos + 1];

		if (next != '\t')
			break;

		pos++;
	}

	return Token{
		Token_Type::Indentation
	};
}

static Token consume_string_literal(const std::string& input, int& pos) {
	int start = pos;

	while (pos + 1 < input.length()) {
		char next = input[pos + 1];

		if (next == '"')
			break;

		pos++;
	}

	// todo: detect if missing end quote lol

	auto str = input.substr(start + 1, pos - start);

	pos++;

	return Token{
		Token_Type::String_Literal,
		{},
		str
	};
}

static Token read_next_token(const std::string& input, int& pos) {
	char ch = input[pos];

	if (isalpha(ch) || ch == '_')
		return consume_identifier(input, pos);
	
	if (isdigit(ch))
		return consume_number(input, pos);

	if (isblank(ch)) {
		// whitespace
	}

	Token_Type type = Token_Type::Unknown;

	char next = input[pos + 1];

	switch (ch) {
	//case '\t': return consume_indentation(input, pos);
	case '"': return consume_string_literal(input, pos);
	case '\n': type = Token_Type::New_Line; break;
	case '=':
		if (next == '=') {
			pos++;
			type = Token_Type::Equals;
		} else {
			type = Token_Type::Assignment;
		}
		break;
	case '>':
		if (next == '=') {
			pos++;
			type = Token_Type::Greater_Than_Equals;
		} else {
			type = Token_Type::Greater_Than;
		}
		break;
	case '<':
		if (next == '=') {
			pos++;
			type = Token_Type::Less_Than_Equals;
		} else {
			type = Token_Type::Less_Than;
		}
		break;
	case '+':
		if (next == '=') {
			pos++;
			type = Token_Type::Plus_Equals;
		} else if (next == '+') {
			pos++;
			type = Token_Type::Increment;
		} else {
			type = Token_Type::Plus;
		}
		break;
	case '-':
		if (next == '=') {
			pos++;
			type = Token_Type::Minus_Equals;
		} else if (next == '-') {
			pos++;
			type = Token_Type::Decrement;
		} else {
			type = Token_Type::Minus;
		}
		break;
	case '*':
		if (next == '=') {
			pos++;
			type = Token_Type::Multiply_Equals;
		} else {
			type = Token_Type::Multiply;
		}
		break;
	case '/':
		if (next == '=') {
			pos++;
			type = Token_Type::Divide_Equals;
		} else {
			type = Token_Type::Divide;
		}
		break;
	case '(': type = Token_Type::Open_Parenthesis; break;
	case ')': type = Token_Type::Closed_Parenthesis; break;
	case ',': type = Token_Type::Comma; break;
	case '[': type = Token_Type::Open_Bracket; break;
	case ']': type = Token_Type::Closed_Bracket; break;
	case '{': type = Token_Type::Open_Curly; break;
	case '}': type = Token_Type::Closed_Curly; break;
	case ';': type = Token_Type::Semicolon; break;
	case '.': type = Token_Type::Dot; break;
	case '!':
		if (next == '=') {
			pos++;
			type = Token_Type::Not_Equals; break;
		} else {
			//type = Token_Type::Not; break;
		}
	}

	auto str = input.substr(pos, 1);

	return Token{ type, {}, str };
}

std::vector<Token> Lexer::lex(const std::string& input) {
	std::vector<Token> tokens;
	int pos = 0;
	int line = 0;

	while (pos < input.length()) {
		// multiline comment
		if (input[pos] == '/' && input[pos + 1] == '*') {
			pos += 2;
			while (pos <= input.length()) {
				if (input[pos] == '\n')
					line++;

				if (input[pos] == '*' && input[pos + 1] == '/') {
					pos += 2;
					break;
				}
				pos++;
			}
		}

		// single line comment
		if (input[pos] == '/' && input[pos + 1] == '/') {
			pos += 2;
			while (pos <= input.length() && input[pos] != '\n') {
				pos++;
			}
		}

		Token token = read_next_token(input, pos);
		token.src_info.line = line;
		if (token.type != Token_Type::Unknown && token.type != Token_Type::New_Line)
			tokens.push_back(token);

		if (token.type == Token_Type::New_Line)
			line++;

		pos++;
	}

	tokens.push_back(Token{ Token_Type::End_Of_File });

	return tokens;
}

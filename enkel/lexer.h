#pragma once

#include "token.h"

#include <vector>
#include <string>

namespace Lexer {
	std::vector<Token> lex(const std::string& input);
};

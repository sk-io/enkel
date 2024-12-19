#pragma once

#include "Value.h"

#include <string>

enum class Token_Type {
    Unknown,
    Any,
    Identifier,
    Assignment,
    Number_Literal,
    Boolean_Literal,
    String_Literal,
    Indentation,
    Keyword_If,
    Keyword_Func,
    Keyword_True,
    Keyword_False,
    Keyword_Pass,
    Keyword_While,
    Keyword_For,
    Keyword_In,
    Keyword_Continue,
    Keyword_Return,
    Keyword_Break,
    Keyword_Var,
    Keyword_Class,
    Keyword_Constructor,
    Keyword_This,
    Keyword_New,
    Keyword_Extends,
    Keyword_Is,
    Plus,
    Minus,
    Multiply,
    Divide,
    Greater_Than,
    Greater_Than_Equals,
    Less_Than,
    Less_Than_Equals,
    Equals,
    Not_Equals,
    Not,
    Open_Parenthesis,
    Closed_Parenthesis,
    Comma,
    Semicolon,
    Open_Bracket,
    Closed_Bracket,
    Open_Curly,
    Closed_Curly,
    Dot,
    Plus_Equals,
    Minus_Equals,
    Multiply_Equals,
    Divide_Equals,
    New_Line,
    End_Of_File,
};

struct Token {
	Token_Type type;
    Value value;
    std::string str;
};

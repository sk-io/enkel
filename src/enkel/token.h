#pragma once

#include "value.h"
#include "source_info.h"

#include <string>
#include <stdint.h>

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
    Keyword_Else,
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
    Keyword_Null,
    Keyword_Not,
    Keyword_And,
    Keyword_Or,
    Keyword_Import,
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
    //Not, // should we have both ! and "not"?
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
    Increment,
    Decrement,
    New_Line,
    End_Of_File,
};

struct Token {
	Token_Type type;
    Value value;
    std::string str;
    Source_Info src_info;
};

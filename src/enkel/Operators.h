#pragma once

#include "token.h"

#include <assert.h>

enum class Unary_Op {
    Increment,
    Decrement,
    Not,
};

enum class Bin_Op {
    Not_A_Bin_Op,
    Add,
    Sub,
    Mul,
    Div,
    Assign,
    Equals,
    Not_Equals,
    Greater_Than,
    Greater_Than_Equals,
    Less_Than,
    Less_Than_Equals,
    Dot,
    Is,
    Add_Assign,
    Sub_Assign,
    Mul_Assign,
    Div_Assign,
    And,
    Or,
};

static constexpr Bin_Op get_bin_op(Token_Type type) {
    switch (type) {
    case Token_Type::Assignment: return Bin_Op::Assign;
    case Token_Type::Plus: return Bin_Op::Add;
    case Token_Type::Minus: return Bin_Op::Sub;
    case Token_Type::Multiply: return Bin_Op::Mul;
    case Token_Type::Divide: return Bin_Op::Div;
    case Token_Type::Equals: return Bin_Op::Equals;
    case Token_Type::Not_Equals: return Bin_Op::Not_Equals;
    case Token_Type::Greater_Than: return Bin_Op::Greater_Than;
    case Token_Type::Greater_Than_Equals: return Bin_Op::Greater_Than_Equals;
    case Token_Type::Less_Than: return Bin_Op::Less_Than;
    case Token_Type::Less_Than_Equals: return Bin_Op::Less_Than_Equals;
    case Token_Type::Dot: return Bin_Op::Dot;
    case Token_Type::Keyword_Is: return Bin_Op::Is;
    case Token_Type::Plus_Equals: return Bin_Op::Add_Assign;
    case Token_Type::Minus_Equals: return Bin_Op::Sub_Assign;
    case Token_Type::Multiply_Equals: return Bin_Op::Mul_Assign;
    case Token_Type::Divide_Equals: return Bin_Op::Div_Assign;
    case Token_Type::Keyword_And: return Bin_Op::And;
    case Token_Type::Keyword_Or: return Bin_Op::Or;
    }

    return Bin_Op::Not_A_Bin_Op;
}

static constexpr int get_precedence(Bin_Op op) {
    switch (op) {
    case Bin_Op::Assign:
    case Bin_Op::Add_Assign:
    case Bin_Op::Sub_Assign:
    case Bin_Op::Mul_Assign:
    case Bin_Op::Div_Assign:
        return 1;
    case Bin_Op::Or:
        return 2;
    case Bin_Op::And:
        return 3;
    case Bin_Op::Equals:
    case Bin_Op::Not_Equals:
        return 4;
    case Bin_Op::Greater_Than:
    case Bin_Op::Greater_Than_Equals:
    case Bin_Op::Less_Than:
    case Bin_Op::Less_Than_Equals:
        return 5;
    case Bin_Op::Add:
    case Bin_Op::Sub:
        return 6;
    case Bin_Op::Mul:
    case Bin_Op::Div:
        return 7;
    case Bin_Op::Is:
        return 8;
    case Bin_Op::Dot:
        return 9;
    }
    
    assert(false);
    return -1;
}

static constexpr bool is_left_associative(Bin_Op op) {
    switch (op) {
    case Bin_Op::Assign:
    case Bin_Op::Add_Assign:
    case Bin_Op::Sub_Assign:
    case Bin_Op::Mul_Assign:
    case Bin_Op::Div_Assign:
        return false;
    }
    return true;
}

#include "Parser.h"

#include <assert.h>
#include <iostream>

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
    }

    assert(false);
    return Bin_Op::Add;
}

static constexpr int get_precedence(Bin_Op op) {
    switch (op) {
    case Bin_Op::Assign:
        return 1;
    case Bin_Op::Equals:
    case Bin_Op::Not_Equals:
        return 2;
    case Bin_Op::Greater_Than:
    case Bin_Op::Greater_Than_Equals:
    case Bin_Op::Less_Than:
    case Bin_Op::Less_Than_Equals:
        return 3;
    case Bin_Op::Add:
    case Bin_Op::Sub:
        return 4;
    case Bin_Op::Mul:
    case Bin_Op::Div:
        return 5;
    case Bin_Op::Is:
        return 6;
    case Bin_Op::Dot:
        return 7;
    }
    assert(false);
    return -1;
}

static constexpr bool is_left_associative(Bin_Op op) {
    switch (op) {
    case Bin_Op::Assign: return false;
    }
    return true;
}

std::unique_ptr<AST_Node> Parser::parse() {
    std::unique_ptr<AST_Block> block = std::make_unique<AST_Block>();

    while (peek().type != Token_Type::End_Of_File) {
        block->statements.push_back(parse_statement());
    }

    return block;
}

std::unique_ptr<AST_Node> Parser::parse_block() {
    eat(Token_Type::Open_Curly);
    std::unique_ptr<AST_Block> block = std::make_unique<AST_Block>();

    while (peek().type != Token_Type::Closed_Curly) {
        block->statements.push_back(parse_statement());
    }
    eat(Token_Type::Closed_Curly);

    return block;
}

std::unique_ptr<AST_Node> Parser::parse_statement() {
    if (peek().type == Token_Type::Keyword_Var) {
        return parse_var_decl();
    }

    if (peek().type == Token_Type::Keyword_Func) {
        return parse_func_decl();
    }

    if (peek().type == Token_Type::Keyword_Class) {
        return parse_class_decl();
    }

    // if statement
    if (peek().type == Token_Type::Keyword_If) {
        eat(Token_Type::Keyword_If);
        eat(Token_Type::Open_Parenthesis);
        auto cond = parse_expression();
        eat(Token_Type::Closed_Parenthesis);
        auto body = parse_block();
        return std::make_unique<AST_Conditional>(AST_Node_Type::If, std::move(cond), std::move(body));
    }

    // while loop
    if (peek().type == Token_Type::Keyword_While) {
        eat(Token_Type::Keyword_While);
        eat(Token_Type::Open_Parenthesis);
        auto cond = parse_expression();
        eat(Token_Type::Closed_Parenthesis);
        auto body = parse_block();
        return std::make_unique<AST_Conditional>(AST_Node_Type::While, std::move(cond), std::move(body));
    }

    // for loop
    if (peek().type == Token_Type::Keyword_For) {
        eat(Token_Type::Keyword_For);
        eat(Token_Type::Open_Parenthesis);
        eat(Token_Type::Keyword_Var);

        const std::string& name = eat(Token_Type::Identifier).str;

        eat(Token_Type::Keyword_In);

        auto expr = parse_expression();

        eat(Token_Type::Closed_Parenthesis);

        auto body = parse_block();
        return std::make_unique<AST_For>(name, std::move(expr), std::move(body));
    }

    // return statement
    if (peek().type == Token_Type::Keyword_Return) {
        eat(Token_Type::Keyword_Return);
        auto expr = parse_expression();
        eat(Token_Type::Semicolon);

        return std::make_unique<AST_Return>(std::move(expr));
    }

    // expression
    auto expr = parse_expression();
    eat(Token_Type::Semicolon);
    return expr;
}

std::unique_ptr<AST_Node> Parser::parse_expression() {
    return parse_infix(0);
}

// https://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
std::unique_ptr<AST_Node> Parser::parse_infix(int min_prec) {
    std::unique_ptr<AST_Node> result = parse_postfix();

    while (true) {
        const Token& token = peek(0);
        if (!Token::is_bin_op(token.type))
            break;

        Bin_Op op = get_bin_op(token.type);
        int prec = get_precedence(op);

        if (prec < min_prec)
            break;

        int next_min_prec = prec;
        if (is_left_associative(op)) {
            next_min_prec++;
        }

        eat(token.type);

        std::unique_ptr<AST_Node> rhs = parse_infix(next_min_prec);
        std::unique_ptr<AST_Node> new_result = std::make_unique<AST_Bin_Op>(std::move(result), std::move(rhs), op);
        result = std::move(new_result);
    }

    return result;
}

std::unique_ptr<AST_Node> Parser::parse_postfix() {
    auto result = parse_primary(); // apsldoiasjdhuasd

    while (true) {
        // function call
        if (peek().type == Token_Type::Open_Parenthesis) {
            auto func_call = std::make_unique<AST_Func_Call>(std::move(result));

            eat(Token_Type::Open_Parenthesis);
            if (peek().type != Token_Type::Closed_Parenthesis) {
                while (true) {
                    func_call->args.push_back(parse_expression());

                    if (peek().type == Token_Type::Closed_Parenthesis)
                        break;

                    eat(Token_Type::Comma);
                }
            }
            eat(Token_Type::Closed_Parenthesis);

            result = std::move(func_call);
        } else if (peek().type == Token_Type::Open_Bracket) {
            eat(Token_Type::Open_Bracket);
            auto subscript = parse_expression();
            eat(Token_Type::Closed_Bracket);

            auto node = std::make_unique<AST_Subscript>(std::move(result), std::move(subscript));
            result = std::move(node);
        } else {
            break;
        }
    }

    return result;
}

std::unique_ptr<AST_Node> Parser::parse_primary() {
    // parenthesized expression
    if (peek().type == Token_Type::Open_Parenthesis) {
        eat(Token_Type::Open_Parenthesis);
        auto expr = parse_expression();
        eat(Token_Type::Closed_Parenthesis);
        return expr;
    }

    // identifier
    if (peek().type == Token_Type::Identifier) {
        const Token& token = eat(Token_Type::Identifier);
        return std::unique_ptr<AST_Node>(new AST_Var(token.str));
    }

    // string literal
    if (peek().type == Token_Type::String_Literal) {
        const Token& token = eat(Token_Type::String_Literal);
        return std::make_unique<AST_String_Literal>(token.str);
    }

    // number or bool literal
    if (peek().type == Token_Type::Number_Literal || peek().type == Token_Type::Boolean_Literal) {
        const Token& token = eat(peek().type);
        return std::make_unique<AST_Literal>(token.value);
    }

    // array initializer
    if (peek().type == Token_Type::Open_Bracket) {
        eat(Token_Type::Open_Bracket);

        std::vector<std::unique_ptr<AST_Node>> items;

        if (peek().type != Token_Type::Closed_Bracket) {
            while (true) {
                items.push_back(parse_expression());

                if (peek().type == Token_Type::Closed_Bracket)
                    break;

                eat(Token_Type::Comma); // TODO: allow ending comma?
            }
        }

        eat(Token_Type::Closed_Bracket);
        auto arr_init = std::make_unique<AST_Array_Init>();
        arr_init->items = std::move(items);
        return arr_init;
    }

    if (peek().type == Token_Type::Keyword_This) {
        eat(Token_Type::Keyword_This);
        return std::make_unique<AST_Implied>(AST_Node_Type::This);
    }

    if (peek().type == Token_Type::Keyword_New) {
        eat(Token_Type::Keyword_New);

        std::string name = eat(Token_Type::Identifier).str;

        auto node = std::make_unique<AST_New>(name);

        eat(Token_Type::Open_Parenthesis);
        if (peek().type != Token_Type::Closed_Parenthesis) {
            while (true) {
                node->args.push_back(parse_expression());

                if (peek().type == Token_Type::Closed_Parenthesis)
                    break;

                eat(Token_Type::Comma);
            }
        }
        eat(Token_Type::Closed_Parenthesis);
        return node;
    }

    const Token& token = peek();
    assert(false);
    return nullptr;
}

std::unique_ptr<AST_Node> Parser::parse_var_decl() {
    eat(Token_Type::Keyword_Var);

    std::string name = eat(Token_Type::Identifier).str;

    std::unique_ptr<AST_Node> init;

    if (peek().type == Token_Type::Assignment) {
        eat(Token_Type::Assignment);
        init = parse_expression();
    }

    eat(Token_Type::Semicolon);
    return std::make_unique<AST_Var_Decl>(name, std::move(init));
}

std::unique_ptr<AST_Node> Parser::parse_func_decl() {
    eat(Token_Type::Keyword_Func);
    std::string name = eat(Token_Type::Identifier).str;
    auto func_decl = std::make_unique<AST_Func_Decl>(name, nullptr);
    eat(Token_Type::Open_Parenthesis);
    if (peek().type != Token_Type::Closed_Parenthesis) {
        while (true) {
            const Token& arg_token = eat(Token_Type::Identifier);
            Definition def{};
            def.name = arg_token.str;
            func_decl->args.push_back(def);

            if (peek().type == Token_Type::Closed_Parenthesis)
                break;

            eat(Token_Type::Comma);
        }
    }
    eat(Token_Type::Closed_Parenthesis);
    func_decl->body = parse_block();
    return func_decl;
}

std::unique_ptr<AST_Node> Parser::parse_class_decl() {
    eat(Token_Type::Keyword_Class);
    std::string name = eat(Token_Type::Identifier).str;
    std::string parent;

    if (peek().type == Token_Type::Keyword_Extends) {
        eat(Token_Type::Keyword_Extends);
        parent = eat(Token_Type::Identifier).str;
    }

    eat(Token_Type::Open_Curly);

    auto class_decl = std::make_unique<AST_Class_Decl>(name, parent);

    while (peek().type != Token_Type::Closed_Curly) {
        const Token& next = peek();

        switch (next.type) {
        case Token_Type::Keyword_Var:
            class_decl->members.push_back(parse_var_decl());
            break;
        case Token_Type::Keyword_Func:
            class_decl->members.push_back(parse_func_decl());
            break;
        default:
            assert(false);
        }
    }
    eat(Token_Type::Closed_Curly);

    return class_decl;
}

const Token& Parser::peek(int offset) {
    return tokens[pos + offset];
}

const Token& Parser::eat(Token_Type expected) {
    const Token& token = tokens[pos++];
    if (expected != Token_Type::Any && token.type != expected) {
        error("unexpected token");
    }

    return token;
}

void Parser::error(const std::string& msg) const {
    std::cout << "Parser error: " << msg << "\n";
    assert(false);
    exit(1);
}

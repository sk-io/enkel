#include "Parser.h"

#include <assert.h>
#include <iostream>

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
    if (peek().type == Token_Type::Open_Curly) {
        return parse_block();
    }

    if (peek().type == Token_Type::Keyword_Var) {
        return parse_var_decl();
    }

    if (peek().type == Token_Type::Keyword_Func) {
        return parse_func_decl(true);
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
        auto if_body = parse_statement();

        std::unique_ptr<AST_Node> else_body;

        if (peek().type == Token_Type::Keyword_Else) {
            eat(Token_Type::Keyword_Else);

            else_body = parse_statement();
        }

        return std::make_unique<AST_If>(std::move(cond), std::move(if_body), std::move(else_body));
    }

    // while loop
    if (peek().type == Token_Type::Keyword_While) {
        eat(Token_Type::Keyword_While);
        eat(Token_Type::Open_Parenthesis);
        auto cond = parse_expression();
        eat(Token_Type::Closed_Parenthesis);
        auto body = parse_statement();
        return std::make_unique<AST_While>(std::move(cond), std::move(body));
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

        auto body = parse_statement();
        return std::make_unique<AST_For>(name, std::move(expr), std::move(body));
    }

    // return statement
    if (peek().type == Token_Type::Keyword_Return) {
        eat(Token_Type::Keyword_Return);
        auto expr = parse_expression();
        eat(Token_Type::Semicolon);

        return std::make_unique<AST_Return>(std::move(expr));
    }

    // break
    if (peek().type == Token_Type::Keyword_Break) {
        eat(Token_Type::Keyword_Break);
        eat(Token_Type::Semicolon);

        return std::make_unique<AST_Implied>(AST_Node_Type::Break);
    }

    // continue
    if (peek().type == Token_Type::Keyword_Continue) {
        eat(Token_Type::Keyword_Continue);
        eat(Token_Type::Semicolon);

        return std::make_unique<AST_Implied>(AST_Node_Type::Continue);
    }

    // expression
    auto expr = parse_expression();
    eat(Token_Type::Semicolon);
    return expr;
}

std::unique_ptr<AST_Node> Parser::parse_expression() {
    return parse_infix(0);
}

// TODO: mixed infix and unary ops
// https://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
std::unique_ptr<AST_Node> Parser::parse_infix(int min_prec) {
    std::unique_ptr<AST_Node> result = parse_postfix();

    while (true) {
        const Token& token = peek(0);

        Bin_Op op = get_bin_op(token.type);
        if (op == Bin_Op::Not_A_Bin_Op)
            break;

        int prec = get_precedence(op);

        if (prec < min_prec)
            break;

        int next_min_prec = prec;
        if (is_left_associative(op)) {
            next_min_prec++;
        }

        eat();

        std::unique_ptr<AST_Node> rhs = parse_infix(next_min_prec);
        std::unique_ptr<AST_Node> new_result = std::make_unique<AST_Bin_Op>(std::move(result), std::move(rhs), op);
        result = std::move(new_result);
    }

    return result;
}

std::unique_ptr<AST_Node> Parser::parse_postfix() {
    auto result = parse_primary();

    while (true) {
        if (peek().type == Token_Type::Open_Parenthesis) {
            // function call
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
            // array/table subscript
            eat(Token_Type::Open_Bracket);
            auto subscript = parse_expression();
            eat(Token_Type::Closed_Bracket);

            auto node = std::make_unique<AST_Subscript>(std::move(result), std::move(subscript));
            result = std::move(node);
        } else if (peek().type == Token_Type::Increment || peek().type == Token_Type::Decrement) {
            // ++ or --
            Unary_Op op = eat().type == Token_Type::Increment ? Unary_Op::Increment : Unary_Op::Decrement;

            auto node = std::make_unique<AST_Unary_Op>(std::move(result), op);
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

    // null
    if (peek().type == Token_Type::Keyword_Null) {
        eat(Token_Type::Keyword_Null);
        return std::make_unique<AST_Implied>(AST_Node_Type::Null);
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

    // this
    if (peek().type == Token_Type::Keyword_This) {
        eat(Token_Type::Keyword_This);
        return std::make_unique<AST_Implied>(AST_Node_Type::This);
    }

    // new Foo()
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

std::unique_ptr<AST_Node> Parser::parse_func_decl(bool is_global) {
    eat(Token_Type::Keyword_Func);

    std::string name = eat(Token_Type::Identifier).str;
    auto func_decl = std::make_unique<AST_Func_Decl>(name, nullptr, is_global);

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
            class_decl->members.push_back(parse_func_decl(false));
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
    if (error_callback != nullptr) {
        error_callback(msg);
    } else {
        std::cout << "Parser error: " << msg << "\n";
        assert(false);
        exit(1);
    }
}

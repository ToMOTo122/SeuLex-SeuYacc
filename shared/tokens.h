#ifndef TOKENS_H
#define TOKENS_H

#include <string>

enum class TokenType {
    // identifiers & literals
    NAME, NUMBER,
    // keywords
    INT, FLOAT, VOID, CHAR, STRUCT, ENUM, TYPEDEF,
    IF, ELSE, WHILE, DO, FOR, RETURN, BREAK, CONTINUE,
    // operators
    PLUS, MINUS, MUL, DIV, MOD, ASSIGN,
    EQ, NE, LE, GE, LT, GT,
    AND, OR, INC, DEC, NOT,
    // delimiters
    SEMICOLON, COMMA, DOT,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACK, RBRACK,
    // special
    END_OF_FILE, ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;

    // semantic value
    union {
        int intVal;
        float floatVal;
        char* strVal;
    } attr;
};

inline const char* tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::NAME:    return "NAME";
        case TokenType::NUMBER:  return "NUMBER";
        case TokenType::INT:     return "INT";
        case TokenType::FLOAT:   return "FLOAT";
        case TokenType::VOID:    return "VOID";
        case TokenType::CHAR:    return "CHAR";
        case TokenType::STRUCT:  return "STRUCT";
        case TokenType::IF:      return "IF";
        case TokenType::ELSE:    return "ELSE";
        case TokenType::WHILE:   return "WHILE";
        case TokenType::RETURN:  return "RETURN";
        case TokenType::PLUS:    return "PLUS";
        case TokenType::MINUS:   return "MINUS";
        case TokenType::MUL:     return "MUL";
        case TokenType::DIV:     return "DIV";
        case TokenType::ASSIGN:  return "ASSIGN";
        case TokenType::EQ:      return "EQ";
        case TokenType::NE:      return "NE";
        case TokenType::LE:      return "LE";
        case TokenType::GE:      return "GE";
        case TokenType::LT:      return "LT";
        case TokenType::GT:      return "GT";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA:   return "COMMA";
        case TokenType::DOT:     return "DOT";
        case TokenType::LPAREN:  return "LPAREN";
        case TokenType::RPAREN:  return "RPAREN";
        case TokenType::LBRACE:  return "LBRACE";
        case TokenType::RBRACE:  return "RBRACE";
        case TokenType::LBRACK:  return "LBRACK";
        case TokenType::RBRACK:  return "RBRACK";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}

#endif

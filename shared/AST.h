#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

enum class ASTNodeType {
    PROGRAM, FUNC_DECL, VAR_DECL, PARAM_LIST,
    BLOCK, STMT_LIST,
    IF_STMT, IF_ELSE_STMT, WHILE_STMT, FOR_STMT,
    RETURN_STMT, ASSIGN_STMT,
    BINARY_EXPR, UNARY_EXPR, CALL_EXPR,
    ARRAY_ACCESS, DOT_OP, PTR_OP,
    PRIMARY_EXP,
    TYPE_SPEC
};

struct ASTNode {
    ASTNodeType type;
    std::string name;
    std::string tokenLexeme;
    int line;
    ASTNode* parent;
    std::vector<ASTNode*> children;

    ASTNode(ASTNodeType t) : type(t), line(0), parent(nullptr) {}
    ASTNode(ASTNodeType t, const std::string& n) : type(t), name(n), line(0), parent(nullptr) {}

    void addChild(ASTNode* child) {
        if (child) {
            child->parent = this;
            children.push_back(child);
        }
    }

    ~ASTNode() {
        for (auto* c : children) delete c;
    }
};

inline const char* astNodeTypeName(ASTNodeType t) {
    switch (t) {
        case ASTNodeType::PROGRAM:    return "PROGRAM";
        case ASTNodeType::FUNC_DECL:  return "FUNC_DECL";
        case ASTNodeType::VAR_DECL:   return "VAR_DECL";
        case ASTNodeType::PARAM_LIST: return "PARAM_LIST";
        case ASTNodeType::BLOCK:      return "BLOCK";
        case ASTNodeType::STMT_LIST:  return "STMT_LIST";
        case ASTNodeType::IF_STMT:    return "IF_STMT";
        case ASTNodeType::IF_ELSE_STMT: return "IF_ELSE_STMT";
        case ASTNodeType::WHILE_STMT: return "WHILE_STMT";
        case ASTNodeType::RETURN_STMT: return "RETURN_STMT";
        case ASTNodeType::ASSIGN_STMT: return "ASSIGN_STMT";
        case ASTNodeType::BINARY_EXPR: return "BINARY_EXPR";
        case ASTNodeType::UNARY_EXPR:  return "UNARY_EXPR";
        case ASTNodeType::CALL_EXPR:   return "CALL_EXPR";
        case ASTNodeType::ARRAY_ACCESS: return "ARRAY_ACCESS";
        case ASTNodeType::DOT_OP:      return "DOT_OP";
        case ASTNodeType::PRIMARY_EXP: return "PRIMARY_EXP";
        default: return "UNKNOWN";
    }
}

inline void printAST(ASTNode* node, int depth = 0) {
    if (!node) return;
    for (int i = 0; i < depth; i++) std::cout << "  ";
    std::cout << astNodeTypeName(node->type);
    if (!node->name.empty()) std::cout << " [" << node->name << "]";
    if (!node->tokenLexeme.empty()) std::cout << " '" << node->tokenLexeme << "'";
    std::cout << std::endl;
    for (auto* child : node->children) {
        printAST(child, depth + 1);
    }
}

#endif

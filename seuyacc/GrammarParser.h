#ifndef GRAMMARPARSER_H
#define GRAMMARPARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <iostream>

struct GrammarSymbol {
    int id;
    std::string name;
    bool isTerminal;
    int precedence;
    enum Assoc { NONE, LEFT, RIGHT } assoc;

    GrammarSymbol() : id(-1), isTerminal(false), precedence(0), assoc(NONE) {}
};

struct Production {
    int id;
    int lhs;                    // symbol ID of left-hand side
    std::vector<int> rhs;       // symbol IDs of right-hand side
    std::string actionCode;     // semantic action code
    int precedence;             // precedence level (from %left/%right or last terminal)

    Production() : id(-1), lhs(-1), precedence(0) {}
};

struct GrammarSpec {
    std::string userCode;
    std::unordered_map<std::string, int> symbolMap;
    std::vector<GrammarSymbol> symbols;
    std::vector<Production> productions;
    std::string userFunctions;

    // Token declarations
    std::vector<std::string> tokenNames;

    // Union type declarations
    std::string unionCode;
    std::unordered_map<std::string, std::string> typeTags; // symbol → type tag
    std::string startSymbol;
};

GrammarSpec parseGrammarFile(const std::string& filepath);

#endif

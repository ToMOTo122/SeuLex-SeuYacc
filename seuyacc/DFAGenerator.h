#ifndef DFAGENERATOR_H
#define DFAGENERATOR_H

#include "GrammarParser.h"
#include <set>
#include <map>
#include <vector>

// LR(1) Item: [A → α·β, lookahead]
struct LR1Item {
    int prodId;
    int dotPos;
    int lookahead; // terminal symbol id

    bool operator<(const LR1Item& other) const {
        if (prodId != other.prodId) return prodId < other.prodId;
        if (dotPos != other.dotPos) return dotPos < other.dotPos;
        return lookahead < other.lookahead;
    }
    bool operator==(const LR1Item& other) const {
        return prodId == other.prodId && dotPos == other.dotPos && lookahead == other.lookahead;
    }
};

// LR(1) State: set of LR(1) items
struct LR1State {
    int id;
    std::set<LR1Item> items;
    std::map<int, int> transitions; // symbol id → target state id
};

// LALR(1) state: merged LR(1) states with same core
struct LALR1State {
    int id;
    std::set<LR1Item> items;    // items with merged lookaheads
    std::map<int, int> transitions;
    std::set<int> mergedFrom;   // LR(1) state ids merged into this
};

struct LALR1DFA {
    std::vector<LALR1State> states;
    int startState;
};

// Action table entry
enum ActionType { SHIFT, REDUCE, ACCEPT, ERROR };
struct ActionEntry {
    ActionType type;
    int value;
};

// Parse tables
struct ParseTables {
    std::vector<std::unordered_map<int, ActionEntry>> action; // state → terminal → action
    std::vector<std::unordered_map<int, int>> gotoTable;       // state → nonterminal → target
    std::vector<Production> productions;
    std::vector<GrammarSymbol> symbols;
    std::unordered_map<std::string, int> symbolMap;
    GrammarSpec grammar;
};

// FIRST sets
std::map<int, std::set<int>> computeFirstSets(const GrammarSpec& spec);

// FOLLOW sets
std::map<int, std::set<int>> computeFollowSets(const GrammarSpec& spec,
                                                const std::map<int, std::set<int>>& firstSets);

// Build LR(1) DFA
std::vector<LR1State> buildLR1DFA(const GrammarSpec& spec,
                                   const std::map<int, std::set<int>>& firstSets);

// Merge LR(1) → LALR(1)
LALR1DFA mergeLALR1(const std::vector<LR1State>& lr1States);

// Build parse tables from LALR(1) DFA
ParseTables buildParseTables(const LALR1DFA& dfa, const GrammarSpec& spec);

#endif

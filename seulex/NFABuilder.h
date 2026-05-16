#ifndef NFABUILDER_H
#define NFABUILDER_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <string>

struct NFA {
    int startState;
    std::map<int, int> endStatesMap; // end state → rule index
    std::unordered_map<int, std::unordered_multimap<char, int>> states;
    int stateCount;

    NFA() : startState(0), stateCount(0) {}
    int newState() { return stateCount++; }
    void addEdge(int from, char symbol, int to);
};

// Check if a char is a class marker (0x01..0x1F non-whitespace controls)
bool isClassMarker(char c);

// Build NFA for a single postfix RE using Thompson algorithm
NFA buildNFA(const std::string& postfix, int ruleIndex);

// Merge multiple NFAs: create new start with epsilon edges to each rule's NFA
NFA mergeNFAs(const std::vector<NFA>& nfas);

#endif

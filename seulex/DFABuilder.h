#ifndef DFABUILDER_H
#define DFABUILDER_H

#include "NFABuilder.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>

struct DFAState {
    int number;
    std::set<int> nfaSubset;
    std::unordered_map<char, int> edges;
};

struct DFA {
    int startState;
    std::map<int, int> endStatesMap; // DFA state → rule index
    std::vector<DFAState> states;
};

// Epsilon closure: all NFA states reachable via epsilon edges
std::set<int> epsilonClosure(const NFA& nfa, const std::set<int>& states);

// Move: states reachable from set by character c (single step, non-epsilon)
std::set<int> move(const NFA& nfa, const std::set<int>& states, char c);

// Collect all characters used in the NFA
std::set<char> collectAlphabet(const NFA& nfa);

// Subset construction: NFA → DFA
DFA nfaToDFA(const NFA& nfa);

#endif

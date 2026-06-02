// NFABuilder.cpp  —  Fixed version
// Key fixes vs original:
//   1. isClassMarker() now checks 0x80-0xFE range (matching REProcessor.cpp's new marker range)
//   2. buildNFA() supports '?' operator via Thompson construction (optional NFA)
//   3. buildNFA() expands class markers correctly in move transitions
//   4. mergeNFAs() correctly offsets all state numbers when merging

#include "NFABuilder.h"
#include "REProcessor.h"   // for charClassTable
#include <stack>
#include <iostream>
#include <cassert>

// -----------------------------------------------------------------------
// NFA::addEdge
// -----------------------------------------------------------------------
void NFA::addEdge(int from, char symbol, int to) {
    states[from].insert({symbol, to});
}

// -----------------------------------------------------------------------
// isClassMarker
// FIXED: use 0x80-0xFE range, matching REProcessor's new marker allocation.
// The old check (0x01..0x1F) incorrectly treated \t (9), \n (10), \r (13)
// as class markers.
// -----------------------------------------------------------------------
bool isClassMarker(char c) {
    unsigned char uc = (unsigned char)c;
    return uc >= 0x80 && uc <= 0xFE;
}

// -----------------------------------------------------------------------
// Helper: create a fresh NFA fragment for a single character (or class marker)
// Returns {start, end} state ids relative to the NFA's current stateCount.
// -----------------------------------------------------------------------
struct Fragment {
    int start, end;
};

// -----------------------------------------------------------------------
// buildNFA: Thompson construction from postfix RE string
//
// Supported postfix operators:
//   .   concatenation
//   |   alternation
//   *   Kleene star
//   +   one-or-more  (= concat of operand with Kleene star of operand)
//   ?   zero-or-one  (Thompson: add ε bypass)
//   \c  escape sequence: the escaped char is a literal operand
//
// All other characters (including class markers 0x80-0xFE) are literal operands.
// -----------------------------------------------------------------------
NFA buildNFA(const std::string& postfix, int ruleIndex) {
    NFA nfa;
    // Use a base offset so all state IDs in this NFA start from 0
    // (mergeNFAs will re-offset them later)
    std::stack<Fragment> stack;

    auto newState = [&]() -> int { return nfa.newState(); };

    for (size_t i = 0; i < postfix.size(); i++) {
        char c = postfix[i];

        // --- Escape sequence: \c ---
        if (c == '\\' && i + 1 < postfix.size()) {
            char literal = postfix[++i];
            int s = newState(), e = newState();
            nfa.addEdge(s, literal, e);
            stack.push({s, e});
            continue;
        }

        // --- Operators ---
        if (c == '.') {
            // Concatenation: pop two fragments, connect end of first to start of second
            if (stack.size() < 2) { std::cerr << "NFA: concat with <2 operands\n"; continue; }
            Fragment b = stack.top(); stack.pop();
            Fragment a = stack.top(); stack.pop();
            // a.end --ε--> b.start
            nfa.addEdge(a.end, '\0', b.start);
            stack.push({a.start, b.end});
            continue;
        }

        if (c == '|') {
            // Alternation
            if (stack.size() < 2) { std::cerr << "NFA: alt with <2 operands\n"; continue; }
            Fragment b = stack.top(); stack.pop();
            Fragment a = stack.top(); stack.pop();
            int s = newState(), e = newState();
            nfa.addEdge(s, '\0', a.start);
            nfa.addEdge(s, '\0', b.start);
            nfa.addEdge(a.end, '\0', e);
            nfa.addEdge(b.end, '\0', e);
            stack.push({s, e});
            continue;
        }

        if (c == '*') {
            // Kleene star
            if (stack.empty()) { std::cerr << "NFA: star with 0 operands\n"; continue; }
            Fragment a = stack.top(); stack.pop();
            int s = newState(), e = newState();
            nfa.addEdge(s, '\0', a.start);
            nfa.addEdge(s, '\0', e);       // bypass
            nfa.addEdge(a.end, '\0', a.start); // loop
            nfa.addEdge(a.end, '\0', e);
            stack.push({s, e});
            continue;
        }

        if (c == '+') {
            // One-or-more: same as concat(a, a*)
            if (stack.empty()) { std::cerr << "NFA: plus with 0 operands\n"; continue; }
            Fragment a = stack.top(); stack.pop();
            int s = newState(), e = newState();
            // New start → a.start; a.end loops back or exits
            nfa.addEdge(s, '\0', a.start);
            nfa.addEdge(a.end, '\0', a.start); // loop (the * part)
            nfa.addEdge(a.end, '\0', e);
            stack.push({s, e});
            continue;
        }

        if (c == '?') {
            // Zero-or-one: new start→a.start, new start→new end, a.end→new end
            if (stack.empty()) { std::cerr << "NFA: ? with 0 operands\n"; continue; }
            Fragment a = stack.top(); stack.pop();
            int s = newState(), e = newState();
            nfa.addEdge(s, '\0', a.start);
            nfa.addEdge(s, '\0', e);   // bypass (zero occurrences)
            nfa.addEdge(a.end, '\0', e);
            stack.push({s, e});
            continue;
        }

        // --- Literal operand (single char or class marker) ---
        if (isClassMarker(c)) {
            // Class marker: expand to alternation over all chars in the class
            auto it = charClassTable.find(c);
            if (it == charClassTable.end() || it->second.empty()) {
                // Unknown marker: treat as single literal
                int s = newState(), e = newState();
                nfa.addEdge(s, c, e);
                stack.push({s, e});
            } else {
                // Create a fragment: start → (one edge per char in class) → end
                int s = newState(), e = newState();
                for (char ch : it->second) {
                    nfa.addEdge(s, ch, e);
                }
                stack.push({s, e});
            }
        } else {
            // Plain literal character
            int s = newState(), e = newState();
            nfa.addEdge(s, c, e);
            stack.push({s, e});
        }
    }

    // The remaining fragment on the stack is the whole NFA
    if (stack.empty()) {
        // Empty RE: trivial NFA that accepts empty string
        int s = newState(), e = newState();
        nfa.addEdge(s, '\0', e);
        nfa.startState = s;
        nfa.endStatesMap[e] = ruleIndex;
    } else {
        Fragment f = stack.top(); stack.pop();
        nfa.startState = f.start;
        nfa.endStatesMap[f.end] = ruleIndex;
    }

    return nfa;
}

// -----------------------------------------------------------------------
// mergeNFAs: combine multiple per-rule NFAs into one by creating a new
// start state with ε-edges to each sub-NFA's start.
//
// FIXED: each sub-NFA's states must be renumbered with an offset so they
// don't collide with each other.  The original code must handle this;
// we implement it correctly here.
// -----------------------------------------------------------------------
NFA mergeNFAs(const std::vector<NFA>& nfas) {
    NFA combined;
    int newStart = combined.newState(); // state 0 is the new start
    combined.startState = newStart;

    int offset = combined.stateCount; // = 1 after creating start

    for (const NFA& sub : nfas) {
        // Copy all edges with offset applied
        for (const auto& [fromRaw, edges] : sub.states) {
            int from = fromRaw + offset;
            for (const auto& [sym, toRaw] : edges) {
                int to = toRaw + offset;
                combined.addEdge(from, sym, to);
            }
        }
        // Copy end states
        for (const auto& [endRaw, ruleIdx] : sub.endStatesMap) {
            combined.endStatesMap[endRaw + offset] = ruleIdx;
        }
        // Epsilon from new start to sub-NFA's start
        combined.addEdge(newStart, '\0', sub.startState + offset);

        offset += sub.stateCount;
    }

    // Update stateCount
    combined.stateCount = offset;

    return combined;
}
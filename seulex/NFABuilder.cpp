#include "NFABuilder.h"
#include "REProcessor.h"
#include <stack>
#include <iostream>

void NFA::addEdge(int from, char symbol, int to) {
    states[from].insert({symbol, to});
}

bool isClassMarker(char c) {
    unsigned char uc = (unsigned char)c;
    return uc >= 1 && uc <= 31 && c != '\n' && c != '\t' && c != '\r' && c != '\0';
}

static void renumberNFA(NFA& nfa) {
    std::unordered_map<int, int> oldToNew;
    int nextId = 0;
    for (auto& [state, edges] : nfa.states) {
        oldToNew[state] = nextId++;
    }
    NFA result;
    result.stateCount = nextId;
    result.startState = oldToNew[nfa.startState];
    for (auto& [oldState, edges] : nfa.states) {
        int ns = oldToNew[oldState];
        for (auto& [sym, to] : edges) {
            result.addEdge(ns, sym, oldToNew[to]);
        }
    }
    for (auto& [end, ridx] : nfa.endStatesMap) {
        result.endStatesMap[oldToNew[end]] = ridx;
    }
    nfa = result;
}

// Add edges from s1 to s2 for a symbol (which may be a char class marker)
static void addSymbolEdges(NFA& nfa, int s1, char symbol, int s2) {
    if (isClassMarker(symbol)) {
        auto it = charClassTable.find(symbol);
        if (it != charClassTable.end()) {
            for (char c : it->second) {
                nfa.addEdge(s1, c, s2);
            }
        }
    } else if (symbol == '.') {
        // Dot matches any character except newline
        for (int c = 0; c < 256; c++) {
            if (c != '\n') {
                nfa.addEdge(s1, (char)c, s2);
            }
        }
    } else {
        nfa.addEdge(s1, symbol, s2);
    }
}

NFA buildNFA(const std::string& postfix, int ruleIndex) {
    std::stack<NFA> nfaStack;

    for (size_t pi = 0; pi < postfix.size(); pi++) {
        char c = postfix[pi];

        // Handle escaped literal: \c
        if (c == '\\' && pi + 1 < postfix.size()) {
            char literal = postfix[++pi];
            NFA result;
            int s1 = result.newState();
            int s2 = result.newState();
            result.addEdge(s1, literal, s2);
            result.startState = s1;
            result.endStatesMap[s2] = ruleIndex;
            nfaStack.push(result);
            continue;
        }
        if (c == '.' || c == '|') {
            NFA n2 = nfaStack.top(); nfaStack.pop();
            NFA n1 = nfaStack.top(); nfaStack.pop();

            renumberNFA(n1);
            int base1 = 0;
            renumberNFA(n2);
            int base2 = n1.stateCount;
            int totalStates = n1.stateCount + n2.stateCount;

            NFA result;
            result.stateCount = totalStates + 2;

            auto copyWithOffset = [&](const NFA& src, int offset) {
                for (auto& [state, edges] : src.states) {
                    int ns = state + offset;
                    for (auto& [sym, to] : edges) {
                        result.addEdge(ns, sym, to + offset);
                    }
                }
            };

            if (c == '.') {
                int sf = totalStates;
                copyWithOffset(n1, base1);
                copyWithOffset(n2, base2);
                for (auto& [end, ridx] : n1.endStatesMap) {
                    result.addEdge(end + base1, '\0', n2.startState + base2);
                }
                for (auto& [end, ridx] : n2.endStatesMap) {
                    result.addEdge(end + base2, '\0', sf);
                    result.endStatesMap[sf] = ridx;
                }
                result.startState = n1.startState + base1;
            } else { // |
                int s0 = totalStates, sf = totalStates + 1;
                copyWithOffset(n1, base1);
                copyWithOffset(n2, base2);
                result.addEdge(s0, '\0', n1.startState + base1);
                result.addEdge(s0, '\0', n2.startState + base2);
                for (auto& [end, ridx] : n1.endStatesMap) {
                    result.addEdge(end + base1, '\0', sf);
                    result.endStatesMap[sf] = ridx;
                }
                for (auto& [end, ridx] : n2.endStatesMap) {
                    result.addEdge(end + base2, '\0', sf);
                    result.endStatesMap[sf] = ridx;
                }
                result.startState = s0;
            }
            nfaStack.push(result);
        } else if (c == '*') {
            NFA n1 = nfaStack.top(); nfaStack.pop();
            renumberNFA(n1);

            NFA result;
            int s0 = n1.stateCount, sf = n1.stateCount + 1;
            result.stateCount = n1.stateCount + 2;

            for (auto& [state, edges] : n1.states) {
                for (auto& [sym, to] : edges) {
                    result.addEdge(state, sym, to);
                }
            }
            result.addEdge(s0, '\0', n1.startState);
            result.addEdge(s0, '\0', sf);
            for (auto& [end, ridx] : n1.endStatesMap) {
                result.addEdge(end, '\0', n1.startState);
                result.addEdge(end, '\0', sf);
                result.endStatesMap[sf] = ridx;
            }
            result.startState = s0;
            nfaStack.push(result);
        } else {
            // Operand: create s1 → symbol → s2
            NFA result;
            int s1 = result.newState();
            int s2 = result.newState();
            if (c == '$') {
                result.addEdge(s1, '\0', s2);
            } else {
                addSymbolEdges(result, s1, c, s2);
            }
            result.startState = s1;
            result.endStatesMap[s2] = ruleIndex;
            nfaStack.push(result);
        }
    }

    return nfaStack.empty() ? NFA() : nfaStack.top();
}

NFA mergeNFAs(const std::vector<NFA>& nfas) {
    if (nfas.empty()) return NFA();

    NFA result;
    int s0 = 0;
    result.startState = s0;
    int offset = 1; // state 0 is the global start

    for (const auto& nfa : nfas) {
        NFA copy = nfa;
        renumberNFA(copy);

        for (auto& [state, edges] : copy.states) {
            int ns = state + offset;
            for (auto& [sym, to] : edges) {
                result.addEdge(ns, sym, to + offset);
            }
        }
        result.addEdge(s0, '\0', copy.startState + offset);
        for (auto& [end, ridx] : copy.endStatesMap) {
            result.endStatesMap[end + offset] = ridx;
        }
        offset += copy.stateCount;
    }

    result.stateCount = offset;
    return result;
}

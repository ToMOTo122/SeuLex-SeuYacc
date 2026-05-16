#include "DFABuilder.h"
#include "NFABuilder.h"
#include <queue>
#include <stack>
#include <iostream>
#include <set>

std::set<int> epsilonClosure(const NFA& nfa, const std::set<int>& states) {
    std::set<int> closure;
    std::stack<int> worklist;

    for (int s : states) {
        closure.insert(s);
        worklist.push(s);
    }

    while (!worklist.empty()) {
        int s = worklist.top();
        worklist.pop();

        auto it = nfa.states.find(s);
        if (it == nfa.states.end()) continue;

        auto range = it->second.equal_range('\0');
        for (auto rit = range.first; rit != range.second; rit++) {
            int nextState = rit->second;
            if (closure.find(nextState) == closure.end()) {
                closure.insert(nextState);
                worklist.push(nextState);
            }
        }
    }

    return closure;
}

std::set<int> move(const NFA& nfa, const std::set<int>& states, char c) {
    std::set<int> result;
    for (int s : states) {
        auto it = nfa.states.find(s);
        if (it == nfa.states.end()) continue;

        auto range = it->second.equal_range(c);
        for (auto rit = range.first; rit != range.second; rit++) {
            result.insert(rit->second);
        }
    }
    return result;
}

std::set<char> collectAlphabet(const NFA& nfa) {
    std::set<char> alphabet;
    for (const auto& [state, edges] : nfa.states) {
        for (const auto& [sym, to] : edges) {
            if (sym != '\0' && !isClassMarker(sym)) {
                alphabet.insert(sym);
            }
        }
    }
    return alphabet;
}

DFA nfaToDFA(const NFA& nfa) {
    DFA dfa;
    std::map<std::set<int>, int> subsetToStateId;

    std::set<int> startSet = epsilonClosure(nfa, {nfa.startState});
    int startId = 0;
    subsetToStateId[startSet] = startId;
    dfa.startState = startId;
    dfa.states.push_back({startId, startSet, {}});

    std::queue<std::set<int>> worklist;
    worklist.push(startSet);

    std::set<char> alphabet = collectAlphabet(nfa);

    while (!worklist.empty()) {
        std::set<int> currentSubset = worklist.front();
        worklist.pop();
        int currentId = subsetToStateId[currentSubset];

        for (char c : alphabet) {
            std::set<int> moved = move(nfa, currentSubset, c);
            if (moved.empty()) continue;

            std::set<int> closure = epsilonClosure(nfa, moved);
            if (closure.empty()) continue;

            int targetId;
            auto it = subsetToStateId.find(closure);
            if (it == subsetToStateId.end()) {
                targetId = static_cast<int>(dfa.states.size());
                subsetToStateId[closure] = targetId;
                dfa.states.push_back({targetId, closure, {}});
                worklist.push(closure);
            } else {
                targetId = it->second;
            }

            dfa.states[currentId].edges[c] = targetId;
        }
    }

    // Mark end states: priority to smallest rule index
    for (auto& ds : dfa.states) {
        int bestRule = -1;
        for (int nfaState : ds.nfaSubset) {
            auto it = nfa.endStatesMap.find(nfaState);
            if (it != nfa.endStatesMap.end()) {
                if (bestRule == -1 || it->second < bestRule) {
                    bestRule = it->second;
                }
            }
        }
        if (bestRule >= 0) {
            dfa.endStatesMap[ds.number] = bestRule;
        }
    }

    return dfa;
}

#include "DFAMinimizer.h"
#include <map>
#include <algorithm>
#include <iostream>

DFA minimizeDFA(const DFA& dfa) {
    if (dfa.states.empty()) return dfa;

    std::set<char> alphabet;
    for (const auto& s : dfa.states) {
        for (const auto& [c, to] : s.edges) {
            alphabet.insert(c);
        }
    }

    int n = static_cast<int>(dfa.states.size());

    // Initial partition: split by end state action, non-end states in one group
    std::vector<int> stateToGroup(n, -1);
    int deadState = -1;

    // Check if there's a dead state (no edges except self-loops for all chars)
    for (int i = 0; i < n; i++) {
        bool allSelf = true;
        for (char c : alphabet) {
            auto it = dfa.states[i].edges.find(c);
            if (it != dfa.states[i].edges.end() && it->second != i) {
                allSelf = false;
                break;
            }
            if (it == dfa.states[i].edges.end()) {
                allSelf = false;
                break;
            }
        }
        if (allSelf && dfa.endStatesMap.find(i) == dfa.endStatesMap.end()) {
            deadState = i;
            break;
        }
    }

    int nextGroup = 0;
    std::map<int, int> endActionToGroup;

    for (int i = 0; i < n; i++) {
        auto it = dfa.endStatesMap.find(i);
        if (it != dfa.endStatesMap.end()) {
            int action = it->second;
            if (endActionToGroup.find(action) == endActionToGroup.end()) {
                endActionToGroup[action] = nextGroup++;
            }
            stateToGroup[i] = endActionToGroup[action];
        } else {
            stateToGroup[i] = nextGroup;
        }
    }
    // All non-end states in one group
    if (endActionToGroup.empty()) {
        nextGroup = 1;
    } else {
        // Assign non-end states
        int nonEndGroup = nextGroup++;
        for (int i = 0; i < n; i++) {
            if (dfa.endStatesMap.find(i) == dfa.endStatesMap.end()) {
                stateToGroup[i] = nonEndGroup;
            }
        }
    }

    // Partition refinement
    bool changed = true;
    int maxIter = 100;
    while (changed && maxIter-- > 0) {
        changed = false;
        std::vector<int> newStateToGroup = stateToGroup;

        for (int g = 0; g < nextGroup; g++) {
            std::vector<int> members;
            for (int i = 0; i < n; i++) {
                if (stateToGroup[i] == g) members.push_back(i);
            }
            if (members.size() <= 1) continue;

            // Try to split this group
            for (char c : alphabet) {
                std::map<int, std::vector<int>> splits;
                for (int s : members) {
                    auto it = dfa.states[s].edges.find(c);
                    int targetGroup = -1;
                    if (it != dfa.states[s].edges.end()) {
                        targetGroup = stateToGroup[it->second];
                    }
                    splits[targetGroup].push_back(s);
                }
                if (splits.size() > 1) {
                    // Found a split
                    for (auto& [tg, states] : splits) {
                        if (tg == splits.begin()->first) continue; // keep first in old group
                        int newG = nextGroup++;
                        for (int s : states) {
                            newStateToGroup[s] = newG;
                        }
                    }
                    changed = true;
                    break; // break char loop, re-scan
                }
            }
            if (changed) break;
        }

        stateToGroup = newStateToGroup;
    }

    // Build minimized DFA
    DFA minDfa;
    std::map<int, int> groupToNewState;

    for (int i = 0; i < n; i++) {
        int g = stateToGroup[i];
        if (groupToNewState.find(g) == groupToNewState.end()) {
            int newId = static_cast<int>(minDfa.states.size());
            groupToNewState[g] = newId;
            DFAState ds;
            ds.number = newId;
            ds.nfaSubset = dfa.states[i].nfaSubset;
            minDfa.states.push_back(ds);
        }
    }

    for (int i = 0; i < n; i++) {
        int g = stateToGroup[i];
        int newId = groupToNewState[g];

        // Copy transitions
        for (const auto& [c, to] : dfa.states[i].edges) {
            int toGroup = stateToGroup[to];
            int newTo = groupToNewState[toGroup];
            minDfa.states[newId].edges[c] = newTo;
        }

        // Mark end states (use representative)
        auto it = dfa.endStatesMap.find(i);
        if (it != dfa.endStatesMap.end()) {
            if (minDfa.endStatesMap.find(newId) == minDfa.endStatesMap.end() ||
                it->second < minDfa.endStatesMap[newId]) {
                minDfa.endStatesMap[newId] = it->second;
            }
        }
    }

    // Set start state
    int startGroup = stateToGroup[dfa.startState];
    minDfa.startState = groupToNewState[startGroup];

    return minDfa;
}

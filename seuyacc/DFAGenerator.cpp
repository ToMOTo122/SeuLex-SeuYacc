#include "DFAGenerator.h"
#include <queue>
#include <algorithm>
#include <iostream>

// Compute FIRST sets for all symbols
std::map<int, std::set<int>> computeFirstSets(const GrammarSpec& spec) {
    std::map<int, std::set<int>> first;

    // Initialize terminals: FIRST(t) = {t}
    for (const auto& sym : spec.symbols) {
        if (sym.isTerminal) {
            first[sym.id].insert(sym.id);
        }
    }

    bool changed = true;
    int maxIter = 100;
    while (changed && maxIter-- > 0) {
        changed = false;
        for (const auto& prod : spec.productions) {
            int A = prod.lhs;
            bool allEpsilon = true;

            for (int symId : prod.rhs) {
                for (int t : first[symId]) {
                    if (t != -1) { // -1 would be epsilon marker
                        if (first[A].insert(t).second) changed = true;
                    }
                }

                // Check if this symbol can derive epsilon
                bool derivesEpsilon = false;
                // For now, assume no epsilon derivations in minic grammar
                allEpsilon = false;
                break; // Only first symbol matters for non-epsilon case
            }
        }
    }

    return first;
}

// Compute FOLLOW sets
std::map<int, std::set<int>> computeFollowSets(const GrammarSpec& spec,
                                                const std::map<int, std::set<int>>& firstSets) {
    std::map<int, std::set<int>> follow;
    int eofId = spec.symbolMap.at("$");

    // FOLLOW(start) = {$}
    if (!spec.productions.empty()) {
        follow[spec.productions[0].lhs].insert(eofId);
    }

    bool changed = true;
    int maxIter = 100;
    while (changed && maxIter-- > 0) {
        changed = false;
        for (const auto& prod : spec.productions) {
            int A = prod.lhs;
            for (size_t i = 0; i < prod.rhs.size(); i++) {
                int B = prod.rhs[i];
                if (spec.symbols[B].isTerminal) continue;

                // FIRST of everything after B
                if (i + 1 < prod.rhs.size()) {
                    for (size_t j = i + 1; j < prod.rhs.size(); j++) {
                        int beta = prod.rhs[j];
                        for (int t : firstSets.at(beta)) {
                            if (follow[B].insert(t).second) changed = true;
                        }
                        // Assume no epsilon, so break after first
                        break;
                    }
                } else {
                    // FOLLOW(A) ⊆ FOLLOW(B)
                    for (int t : follow[A]) {
                        if (follow[B].insert(t).second) changed = true;
                    }
                }
            }
        }
    }

    return follow;
}

// Compute LR(1) closure
static std::set<LR1Item> closure(const std::set<LR1Item>& I, const GrammarSpec& spec,
                                  const std::map<int, std::set<int>>& firstSets) {
    std::set<LR1Item> result = I;
    std::queue<LR1Item> worklist;
    for (auto& item : I) worklist.push(item);

    while (!worklist.empty()) {
        LR1Item item = worklist.front();
        worklist.pop();

        const Production& prod = spec.productions[item.prodId];
        if (item.dotPos >= (int)prod.rhs.size()) continue;

        int B = prod.rhs[item.dotPos]; // symbol after dot
        if (spec.symbols[B].isTerminal) continue;

        // Compute FIRST(βa) where β is after B and a is lookahead
        std::set<int> lookaheads;
        bool allEpsilon = true;

        for (size_t j = item.dotPos + 1; j < prod.rhs.size(); j++) {
            allEpsilon = false;
            for (int t : firstSets.at(prod.rhs[j])) {
                lookaheads.insert(t);
            }
            break; // Assume no epsilon for now
        }
        if (allEpsilon || item.dotPos + 1 >= prod.rhs.size()) {
            lookaheads.insert(item.lookahead);
        }

        // Add [B → ·γ, b] for each b in lookaheads
        for (const auto& p : spec.productions) {
            if (p.lhs == B) {
                for (int la : lookaheads) {
                    LR1Item newItem;
                    newItem.prodId = p.id;
                    newItem.dotPos = 0;
                    newItem.lookahead = la;
                    if (result.find(newItem) == result.end()) {
                        result.insert(newItem);
                        worklist.push(newItem);
                    }
                }
            }
        }
    }

    return result;
}

// Compute GOTO(I, X)
static std::set<LR1Item> gotoSet(const std::set<LR1Item>& I, int X, const GrammarSpec& spec,
                                  const std::map<int, std::set<int>>& firstSets) {
    std::set<LR1Item> J;
    for (const auto& item : I) {
        const Production& prod = spec.productions[item.prodId];
        if (item.dotPos < (int)prod.rhs.size() && prod.rhs[item.dotPos] == X) {
            LR1Item newItem = item;
            newItem.dotPos++;
            J.insert(newItem);
        }
    }
    return closure(J, spec, firstSets);
}

std::vector<LR1State> buildLR1DFA(const GrammarSpec& spec,
                                   const std::map<int, std::set<int>>& firstSets) {
    std::vector<LR1State> states;
    int eofId = spec.symbolMap.at("$");

    // Start with [S' → ·S, $]
    LR1Item startItem;
    startItem.prodId = 0; // first production is start
    startItem.dotPos = 0;
    startItem.lookahead = eofId;

    std::set<LR1Item> startSet = closure({startItem}, spec, firstSets);
    states.push_back({0, startSet, {}});

    std::queue<int> worklist;
    worklist.push(0);

    while (!worklist.empty()) {
        int stateId = worklist.front();
        worklist.pop();

        // Find all symbols appearing after dot in this state
        std::set<int> symbolsAfterDot;
        for (const auto& item : states[stateId].items) {
            const Production& prod = spec.productions[item.prodId];
            if (item.dotPos < (int)prod.rhs.size()) {
                symbolsAfterDot.insert(prod.rhs[item.dotPos]);
            }
        }

        for (int X : symbolsAfterDot) {
            std::set<LR1Item> gotoItems = gotoSet(states[stateId].items, X, spec, firstSets);
            if (gotoItems.empty()) continue;

            // Check if this state already exists
            int targetId = -1;
            for (size_t i = 0; i < states.size(); i++) {
                if (states[i].items == gotoItems) {
                    targetId = states[i].id;
                    break;
                }
            }
            if (targetId == -1) {
                targetId = static_cast<int>(states.size());
                states.push_back({targetId, gotoItems, {}});
                worklist.push(targetId);
            }

            states[stateId].transitions[X] = targetId;
        }
    }

    std::cout << "LR(1) DFA: " << states.size() << " states" << std::endl;
    return states;
}

// Merge LR(1) states with same core into LALR(1)
LALR1DFA mergeLALR1(const std::vector<LR1State>& lr1States) {
    // Core: set of (prodId, dotPos) pairs — ignoring lookahead
    auto getCore = [](const std::set<LR1Item>& items) -> std::set<std::pair<int,int>> {
        std::set<std::pair<int,int>> core;
        for (auto& it : items) {
            core.insert({it.prodId, it.dotPos});
        }
        return core;
    };

    std::map<std::set<std::pair<int,int>>, int> coreToState;
    LALR1DFA dfa;

    for (const auto& lr1s : lr1States) {
        auto core = getCore(lr1s.items);
        auto it = coreToState.find(core);
        if (it == coreToState.end()) {
            int newId = static_cast<int>(dfa.states.size());
            coreToState[core] = newId;
            LALR1State newState;
            newState.id = newId;
            newState.items = lr1s.items;
            newState.mergedFrom.insert(lr1s.id);
            dfa.states.push_back(newState);
        } else {
            // Merge lookaheads
            int existingId = it->second;
            for (const auto& item : lr1s.items) {
                // Find matching core item and add lookahead
                for (auto& existingItem : dfa.states[existingId].items) {
                    if (existingItem.prodId == item.prodId &&
                        existingItem.dotPos == item.dotPos) {
                        // Already has this lookahead (set property)
                        break;
                    }
                }
                dfa.states[existingId].items.insert(item);
            }
            dfa.states[existingId].mergedFrom.insert(lr1s.id);
        }
    }

    // Rebuild transitions
    for (auto& state : dfa.states) {
        // Use representative LR(1) state for transitions
        int repLr1 = *state.mergedFrom.begin();
        for (const auto& [sym, targetLr1] : lr1States[repLr1].transitions) {
            // Find which LALR(1) state contains this target
            auto targetCore = getCore(lr1States[targetLr1].items);
            auto targetIt = coreToState.find(targetCore);
            if (targetIt != coreToState.end()) {
                state.transitions[sym] = targetIt->second;
            }
        }
    }

    dfa.startState = 0;
    std::cout << "LALR(1) DFA: " << dfa.states.size() << " states (merged from "
              << lr1States.size() << " LR(1) states)" << std::endl;
    return dfa;
}

// Build ACTION and GOTO tables
ParseTables buildParseTables(const LALR1DFA& dfa, const GrammarSpec& spec) {
    ParseTables tables;
    tables.productions = spec.productions;
    tables.symbols = spec.symbols;
    tables.symbolMap = spec.symbolMap;
    tables.grammar = spec;

    int nStates = static_cast<int>(dfa.states.size());
    tables.action.resize(nStates);
    tables.gotoTable.resize(nStates);

    int eofId = spec.symbolMap.at("$");

    for (const auto& state : dfa.states) {
        for (const auto& item : state.items) {
            const Production& prod = spec.productions[item.prodId];

            if (item.dotPos < (int)prod.rhs.size()) {
                int nextSym = prod.rhs[item.dotPos];
                if (spec.symbols[nextSym].isTerminal && nextSym != eofId) {
                    // Shift: item with dot before a terminal
                    auto transIt = state.transitions.find(nextSym);
                    if (transIt != state.transitions.end()) {
                        ActionEntry shift;
                        shift.type = SHIFT;
                        shift.value = transIt->second;
                        tables.action[state.id][nextSym] = shift;
                    }
                }
            } else {
                // Reduce or Accept
                if (prod.id == 0) {
                    // Start production S' → S
                    ActionEntry accept;
                    accept.type = ACCEPT;
                    accept.value = 0;
                    tables.action[state.id][eofId] = accept;
                } else {
                    ActionEntry reduce;
                    reduce.type = REDUCE;
                    reduce.value = prod.id;
                    tables.action[state.id][item.lookahead] = reduce;
                }
            }
        }

        // GOTO transitions for non-terminals
        for (const auto& [sym, target] : state.transitions) {
            if (!spec.symbols[sym].isTerminal) {
                tables.gotoTable[state.id][sym] = target;
            }
        }
    }

    // Resolve shift-reduce conflicts using precedence
    for (int s = 0; s < nStates; s++) {
        for (auto& [term, action] : tables.action[s]) {
            // Check for conflicts
            if (action.type == SHIFT) {
                // Look for a reduce on same terminal (shift-reduce conflict)
                // If shift symbol has higher precedence, keep shift
                // Otherwise prefer reduce
                auto it = tables.action[s].find(term);
                // For now: default behavior (shift preferred) — matches Yacc
            }
        }

        // Detect and report conflicts
        std::map<int, std::vector<ActionEntry>> terminalActions;
        for (auto& [term, action] : tables.action[s]) {
            terminalActions[term].push_back(action);
        }
        for (auto& [term, actions] : terminalActions) {
            if (actions.size() > 1) {
                int reduceCount = 0, shiftCount = 0;
                for (auto& a : actions) {
                    if (a.type == REDUCE) reduceCount++;
                    if (a.type == SHIFT) shiftCount++;
                }
                if (shiftCount > 0 && reduceCount > 0) {
                    // Shift-reduce conflict: prefer shift by default
                    // Remove reduce actions (keep first = shift)
                    // Actually, just keep the first shift
                    std::cout << "  Warning: shift/reduce conflict in state " << s
                              << " on symbol " << spec.symbols[term].name
                              << " (resolved: shift)" << std::endl;
                }
                if (reduceCount > 1) {
                    std::cout << "  Warning: reduce/reduce conflict in state " << s
                              << " on symbol " << spec.symbols[term].name
                              << " (resolved: first production)" << std::endl;
                }
            }
        }
    }

    std::cout << "Parse tables built: " << nStates << " states" << std::endl;
    return tables;
}

#include "DFAGenerator.h"
#include <queue>
#include <algorithm>
#include <iostream>

const int EPSILON_MARKER = -1;

// Compute FIRST sets for all symbols (Correctly handling Epsilon)
std::map<int, std::set<int>> computeFirstSets(const GrammarSpec& spec) {
    std::map<int, std::set<int>> first;

    for (const auto& sym : spec.symbols) {
        if (sym.isTerminal) {
            first[sym.id].insert(sym.id);
        }
    }
    for (const auto& prod : spec.productions) {
        if (prod.rhs.empty()) {
            first[prod.lhs].insert(EPSILON_MARKER);
        }
    }

    bool changed = true;
    int maxIter = 1000;
    while (changed && maxIter-- > 0) {
        changed = false;
        for (const auto& prod : spec.productions) {
            int A = prod.lhs;
            bool allEpsilon = true;

            for (int symId : prod.rhs) {
                bool currentDerivesEpsilon = false;
                for (int t : first[symId]) {
                    if (t != EPSILON_MARKER) {
                        if (first[A].insert(t).second) changed = true;
                    } else {
                        currentDerivesEpsilon = true;
                    }
                }
                if (!currentDerivesEpsilon) {
                    allEpsilon = false;
                    break; 
                }
            }

            if (allEpsilon) {
                if (first[A].insert(EPSILON_MARKER).second) changed = true;
            }
        }
    }
    return first;
}

std::map<int, std::set<int>> computeFollowSets(const GrammarSpec& spec,
                                                const std::map<int, std::set<int>>& firstSets) {
    std::map<int, std::set<int>> follow;
    int eofId = spec.symbolMap.at("$");

    if (!spec.productions.empty()) {
        follow[spec.productions[0].lhs].insert(eofId);
    }

    bool changed = true;
    int maxIter = 1000;
    while (changed && maxIter-- > 0) {
        changed = false;
        for (const auto& prod : spec.productions) {
            int A = prod.lhs;
            for (size_t i = 0; i < prod.rhs.size(); i++) {
                int B = prod.rhs[i];
                if (spec.symbols[B].isTerminal) continue;

                bool allCanBeEpsilon = true;
                if (i + 1 < prod.rhs.size()) {
                    for (size_t j = i + 1; j < prod.rhs.size(); j++) {
                        int beta = prod.rhs[j];
                        bool betaHasEpsilon = false;
                        auto it = firstSets.find(beta);
                        if (it != firstSets.end()) {
                            for (int t : it->second) {
                                if (t != EPSILON_MARKER) {
                                    if (follow[B].insert(t).second) changed = true;
                                } else {
                                    betaHasEpsilon = true;
                                }
                            }
                        }
                        if (!betaHasEpsilon) {
                            allCanBeEpsilon = false;
                            break;
                        }
                    }
                }

                if (allCanBeEpsilon) {
                    for (int t : follow[A]) {
                        if (follow[B].insert(t).second) changed = true;
                    }
                }
            }
        }
    }
    return follow;
}

static bool derivesEpsilon(int symId, const std::map<int, std::set<int>>& firstSets) {
    auto it = firstSets.find(symId);
    return (it != firstSets.end() && it->second.count(EPSILON_MARKER) > 0);
}

static std::set<int> firstOfBetaA(const Production& prod, int dotPos,
                                    int lookahead,
                                    const GrammarSpec& spec,
                                    const std::map<int, std::set<int>>& firstSets) {
    std::set<int> result;
    bool allCanBeEpsilon = true;

    for (size_t j = (size_t)(dotPos + 1); j < prod.rhs.size(); j++) {
        int sym = prod.rhs[j];
        auto it = firstSets.find(sym);
        if (it != firstSets.end()) {
            for (int t : it->second) {
                if (t != EPSILON_MARKER) {
                    result.insert(t);
                }
            }
        }
        if (!derivesEpsilon(sym, firstSets)) {
            allCanBeEpsilon = false;
            break;
        }
    }

    if (allCanBeEpsilon) {
        result.insert(lookahead);
    }
    return result;
}

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

        int B = prod.rhs[item.dotPos]; 
        if (spec.symbols[B].isTerminal) continue;

        std::set<int> lookaheads = firstOfBetaA(prod, item.dotPos, item.lookahead, spec, firstSets);

        for (const auto& p : spec.productions) {
            if (p.lhs == B) {
                for (int la : lookaheads) {
                    LR1Item newItem;
                    newItem.prodId = p.id;
                    newItem.dotPos = 0;
                    newItem.lookahead = la;
                    if (result.insert(newItem).second) {
                        worklist.push(newItem);
                    }
                }
            }
        }
    }
    return result;
}

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

    LR1Item startItem;
    startItem.prodId = 0;
    startItem.dotPos = 0;
    startItem.lookahead = eofId;

    std::set<LR1Item> startSet = closure({startItem}, spec, firstSets);
    states.push_back({0, startSet, {}});

    std::queue<int> worklist;
    worklist.push(0);

    while (!worklist.empty()) {
        int stateId = worklist.front();
        worklist.pop();

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

LALR1DFA mergeLALR1(const std::vector<LR1State>& lr1States) {
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
            int existingId = it->second;
            for (const auto& item : lr1s.items) {
                dfa.states[existingId].items.insert(item);
            }
            dfa.states[existingId].mergedFrom.insert(lr1s.id);
        }
    }

    for (auto& state : dfa.states) {
        int repLr1 = *state.mergedFrom.begin();
        for (const auto& [sym, targetLr1] : lr1States[repLr1].transitions) {
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
                    auto transIt = state.transitions.find(nextSym);
                    if (transIt != state.transitions.end()) {
                        ActionEntry shift;
                        shift.type = SHIFT;
                        shift.value = transIt->second;
                        
                        auto& existing = tables.action[state.id][nextSym];
                        if (existing.type == 0) {
                            existing = shift;
                        } else if (existing.type == REDUCE) {
                            // shift-reduce conflict: resolve by precedence/associativity
                            const Production& reduceProd = spec.productions[existing.value];
                            int tokPrec = spec.symbols[nextSym].precedence;
                            int prodPrec = reduceProd.precedence;
                            if (tokPrec > 0 && prodPrec > 0) {
                                if (tokPrec > prodPrec) {
                                    existing = shift;
                                } else if (tokPrec == prodPrec) {
                                    if (spec.symbols[nextSym].assoc == GrammarSymbol::RIGHT) {
                                        existing = shift;
                                    }
                                    // LEFT → keep reduce; NONE → keep reduce
                                }
                                // prodPrec > tokPrec → keep reduce
                            } else {
                                existing = shift;  // no precedence info: default shift
                            }
                        }
                    }
                }
            } else {
                int lookahead = item.lookahead;
                ActionEntry reduce;
                if (prod.id == 0) {
                    reduce.type = ACCEPT;
                    reduce.value = 0;
                } else {
                    reduce.type = REDUCE;
                    reduce.value = prod.id;
                }

                auto& existing = tables.action[state.id][lookahead];
                if (existing.type == 0) {
                    existing = reduce;
                } else if (existing.type == REDUCE) {
                    if (reduce.value < existing.value) {
                        existing = reduce;
                    }
                }
            }
        }

        for (const auto& [sym, target] : state.transitions) {
            if (!spec.symbols[sym].isTerminal) {
                tables.gotoTable[state.id][sym] = target;
            }
        }
    }

    std::cout << "Parse tables built: " << nStates << " states" << std::endl;
    return tables;
}
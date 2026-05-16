#ifndef DFAMINIMIZER_H
#define DFAMINIMIZER_H

#include "DFABuilder.h"

// Minimize DFA using Hopcroft-like partition refinement
DFA minimizeDFA(const DFA& dfa);

#endif

#include "LexParser.h"
#include "REProcessor.h"
#include "NFABuilder.h"
#include "DFABuilder.h"
#include "DFAMinimizer.h"
#include "LexCodeGen.h"

#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: SeuLex <input.l> [-o output.c]" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = "yylex.c";
    for (int i = 2; i < argc; i++) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            outputPath = argv[++i];
        }
    }

    std::cout << "SeuLex: processing " << inputPath << std::endl;

    // Phase 1: Parse .l file
    std::cout << "[1/6] Parsing .l file..." << std::endl;
    LexSpec spec = parseLexFile(inputPath);
    std::cout << "  -> " << spec.defs.size() << " definitions, "
              << spec.rules.size() << " rules" << std::endl;

    if (spec.rules.empty()) {
        std::cerr << "Error: no rules found in input file." << std::endl;
        return 1;
    }

    // Phase 2: Process REs (expand defs, normalize, add dots, postfix)
    std::cout << "[2/6] Processing regular expressions..." << std::endl;
    std::vector<std::string> rulePostfix;
    int wildcardRule = -1;  // index of wildcard/error rule to handle specially
    int nRules = (int)spec.rules.size();
    // Detect wildcard rule (last rule with pattern "." or single-char match-all)
    if (nRules > 0) {
        std::string lastRe = spec.rules[nRules - 1].regex;
        std::string la = trim(lastRe);
        if (la == "." || la == ".|\n") {
            wildcardRule = nRules - 1;
            nRules--; // exclude from DFA
            std::cout << "  (rule " << wildcardRule << " is wildcard, handling separately)" << std::endl;
        }
    }

    for (size_t i = 0; i < (size_t)nRules; i++) {
        std::string re = spec.rules[i].regex;
        std::cout << "  Rule " << i << ": " << re;

        // Expand {name} references
        re = expandDefs(re, spec.defs);
        std::cout << " -> " << re;

        // Normalize (handle ?, +, [abc], . etc.)
        re = normalizeRE(re);
        std::cout << " -> " << re;

        // Add concatenation dots
        re = addDots(re);
        std::cout << " -> " << re;

        // Convert to postfix
        re = infixToPostfix(re);
        std::cout << " -> " << re << std::endl;

        rulePostfix.push_back(re);
    }

    // Phase 3: Build NFA for each rule and merge
    std::cout << "[3/6] Building NFAs..." << std::endl;
    std::vector<NFA> nfas;
    for (size_t i = 0; i < rulePostfix.size(); i++) {
        NFA nfa = buildNFA(rulePostfix[i], static_cast<int>(i));
        std::cout << "  Rule " << i << " NFA: " << nfa.stateCount << " states" << std::endl;
        nfas.push_back(nfa);
    }

    NFA combinedNFA = mergeNFAs(nfas);
    std::cout << "  Combined NFA: " << combinedNFA.stateCount << " states" << std::endl;

    // Phase 4: NFA → DFA (subset construction)
    std::cout << "[4/6] Converting NFA to DFA..." << std::endl;
    DFA dfa = nfaToDFA(combinedNFA);
    std::cout << "  DFA: " << dfa.states.size() << " states" << std::endl;

    // Phase 5: Minimize DFA
    std::cout << "[5/6] Minimizing DFA..." << std::endl;
    DFA minDfa = minimizeDFA(dfa);
    std::cout << "  Minimized DFA: " << minDfa.states.size() << " states" << std::endl;

    // Phase 6: Generate code
    std::cout << "[6/6] Generating code..." << std::endl;
    generateLexCode(spec, minDfa, rulePostfix, outputPath, wildcardRule);

    std::cout << "Done! Output written to " << outputPath << std::endl;
    return 0;
}

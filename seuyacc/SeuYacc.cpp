#include "GrammarParser.h"
#include "DFAGenerator.h"
#include "Emitter.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: SeuYacc <input.y> [-o output.c]" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = "yyparse.c";
    for (int i = 2; i < argc; i++) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            outputPath = argv[++i];
        }
    }

    std::cout << "SeuYacc: processing " << inputPath << std::endl;

    // Phase 1: Parse grammar file
    std::cout << "[1/4] Parsing grammar file..." << std::endl;
    GrammarSpec spec = parseGrammarFile(inputPath);
    std::cout << "  -> " << spec.symbols.size() << " symbols, "
              << spec.productions.size() << " productions" << std::endl;

    if (spec.productions.empty()) {
        std::cerr << "Error: no grammar rules found." << std::endl;
        return 1;
    }

    // Phase 2: Compute FIRST, FOLLOW, build LR(1) → LALR(1)
    std::cout << "[2/4] Computing FIRST/FOLLOW sets..." << std::endl;
    auto firstSets = computeFirstSets(spec);
    for (const auto& sym : spec.symbols) {
        if (!sym.isTerminal && !firstSets[sym.id].empty()) {
            std::cout << "  FIRST(" << sym.name << ") = {";
            bool first = true;
            for (int t : firstSets[sym.id]) {
                if (!first) std::cout << ", ";
                std::cout << spec.symbols[t].name;
                first = false;
            }
            std::cout << "}" << std::endl;
        }
    }

    auto followSets = computeFollowSets(spec, firstSets);

    std::cout << "[3/4] Building LR(1) DFA → LALR(1)..." << std::endl;
    auto lr1States = buildLR1DFA(spec, firstSets);
    auto lalr1DFA = mergeLALR1(lr1States);

    // Phase 3: Build parse tables
    std::cout << "[4/4] Building parse tables and generating code..." << std::endl;
    ParseTables tables = buildParseTables(lalr1DFA, spec);

    // Phase 4: Generate yyparse.c
    generateYaccCode(tables, outputPath);

    std::cout << "Done! Output written to " << outputPath << std::endl;
    return 0;
}

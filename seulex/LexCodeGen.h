#ifndef LEXCODEGEN_H
#define LEXCODEGEN_H

#include "LexParser.h"
#include "DFAMinimizer.h"
#include <string>

void generateLexCode(const LexSpec& spec, const DFA& dfa,
                     const std::vector<std::string>& rulePostfix,
                     const std::string& outputPath,
                     int wildcardRule = -1);

#endif

#ifndef REPROCESSOR_H
#define REPROCESSOR_H

#include <string>
#include <unordered_map>
#include <set>

// Global table mapping class marker chars (0x01..) to character sets
extern std::unordered_map<char, std::set<char>> charClassTable;
char registerCharClass(const std::set<char>& chars);

// Expand definitions in regex recursively
std::string expandDefs(const std::string& regex,
                       const std::unordered_map<std::string, std::string>& defs);

// Normalize regex: expand ?, +, [...] (compact), "..." into basic ops
std::string normalizeRE(const std::string& re);

// Insert explicit concatenation dots
std::string addDots(const std::string& re);

// Convert infix RE to postfix
std::string infixToPostfix(const std::string& re);

int priority(char c);

// Extract the last complete operand from an RE string
std::string extractLastOperand(const std::string& s);

#endif

#ifndef LEXPARSER_H
#define LEXPARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

struct LexRule {
    std::string regex;
    std::string action;
};

struct LexSpec {
    std::string user_code;
    std::unordered_map<std::string, std::string> defs;
    std::vector<LexRule> rules;
    std::string user_functions;
};

enum class ParseState {
    STATE_DEFINE = 0,
    STATE_RULES = 1,
    STATE_USER_CODE = 2
};

LexSpec parseLexFile(const std::string& filepath);
std::string trim(const std::string& s);
int findActionBrace(const std::string& line);

#endif

#include "GrammarParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <algorithm>

static std::string trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && isspace((unsigned char)s[start])) start++;
    while (end > start && isspace((unsigned char)s[end-1])) end--;
    return s.substr(start, end - start);
}

static std::string extractBraced(const std::string& line, size_t& pos) {
    if (pos >= line.size() || line[pos] != '{') return "";
    int depth = 0;
    while (pos < line.size()) {
        if (line[pos] == '{') depth++;
        else if (line[pos] == '}') {
            depth--;
            if (depth == 0) {
                pos++;
                return line.substr(0, pos); // caller needs the content only
            }
        }
        pos++;
    }
    return "";
}

// Extract content between first { and matching }
static std::string extractBracedContent(const std::string& s) {
    size_t pos = 0;
    while (pos < s.size() && s[pos] != '{') pos++;
    if (pos >= s.size()) return "";
    int depth = 0;
    size_t start = pos + 1;
    while (pos < s.size()) {
        if (s[pos] == '{') depth++;
        else if (s[pos] == '}') {
            depth--;
            if (depth == 0) {
                return s.substr(start, pos - start);
            }
        }
        pos++;
    }
    return "";
}

GrammarSpec parseGrammarFile(const std::string& filepath) {
    GrammarSpec spec;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open " << filepath << std::endl;
        return spec;
    }

    int symbolCounter = 0;
    auto getOrAddSymbol = [&](const std::string& name, bool isTerminal) -> int {
        if (spec.symbolMap.find(name) != spec.symbolMap.end()) {
            return spec.symbolMap[name];
        }
        int id = symbolCounter++;
        spec.symbolMap[name] = id;
        GrammarSymbol gs;
        gs.id = id;
        gs.name = name;
        gs.isTerminal = isTerminal;
        spec.symbols.push_back(gs);
        return id;
    };

    getOrAddSymbol("$", true); // EOF

    enum Section { SEC_DEFINITIONS, SEC_RULES, SEC_USER_CODE };
    Section section = SEC_DEFINITIONS;
    int currentPrecedence = 0;
    bool inUserCodeBlock = false;

    std::string line;
    std::string currentRule; // accumulated rule text (all alternatives)
    int currentLHS = -1;

    auto processRule = [&](const std::string& ruleText, int lhs) {
        // Rule text: "alt1 | alt2 | alt3 ;" (semicolon optional here)
        std::string text = ruleText;
        // Remove trailing semicolon
        if (!text.empty() && text.back() == ';') text.pop_back();
        text = trim(text);
        if (text.empty()) return;

        // Split alternatives by | (outside braces)
        std::vector<std::string> alternatives;
        size_t start = 0;
        int braceDepth = 0;
        for (size_t i = 0; i < text.size(); i++) {
            if (text[i] == '{') braceDepth++;
            else if (text[i] == '}') braceDepth--;
            else if (text[i] == '|' && braceDepth == 0) {
                alternatives.push_back(trim(text.substr(start, i - start)));
                start = i + 1;
            }
        }
        alternatives.push_back(trim(text.substr(start)));

        for (const auto& alt : alternatives) {
            if (alt.empty()) continue;

            Production prod;
            prod.id = static_cast<int>(spec.productions.size());
            prod.lhs = lhs;

            std::string rhsStr = alt;
            std::string action;

            // Extract { action } if present
            size_t actionStart = rhsStr.find('{');
            if (actionStart != std::string::npos) {
                size_t ap = actionStart + 1;
                int adepth = 1;
                while (ap < rhsStr.size() && adepth > 0) {
                    if (rhsStr[ap] == '{') adepth++;
                    else if (rhsStr[ap] == '}') adepth--;
                    ap++;
                }
                action = rhsStr.substr(actionStart + 1, ap - actionStart - 2);
                rhsStr = trim(rhsStr.substr(0, actionStart));
            }

            prod.actionCode = action;

            // Parse RHS symbols by whitespace
            std::istringstream iss(rhsStr);
            std::string symName;
            bool inComment = false;
            while (iss >> symName) {
                if (inComment) {
                    if (symName.find("*/") != std::string::npos) inComment = false;
                    continue;
                }
                if (symName.find("/*") == 0) {
                    if (symName.find("*/") == std::string::npos) inComment = true;
                    continue;
                }
                if (symName == "%prec") {
                    iss >> symName;
                    continue;
                }
                // Remove quotes around single-char terminals
                if (symName.size() == 3 && symName[0] == '\'' && symName[2] == '\'') {
                    symName = symName.substr(1, 1);
                }

                // Determine if terminal
                bool isTerm = !symName.empty() &&
                    (isupper((unsigned char)symName[0]) || symName.size() == 1 && !isalpha((unsigned char)symName[0]));

                int sid = getOrAddSymbol(symName, isTerm);
                prod.rhs.push_back(sid);

                if (spec.symbols[sid].isTerminal && spec.symbols[sid].precedence > 0) {
                    prod.precedence = spec.symbols[sid].precedence;
                }
            }

            spec.productions.push_back(prod);
        }
    };

    while (std::getline(file, line)) {
        std::string trimmed = trim(line);

        // Handle %{ ... %} in definitions
        if (section == SEC_DEFINITIONS) {
            if (trimmed == "%{") { inUserCodeBlock = true; continue; }
            if (trimmed == "%}") { inUserCodeBlock = false; continue; }
            if (inUserCodeBlock) { spec.userCode += line + "\n"; continue; }
        }

        // Section delimiter
        if (trimmed == "%%") {
            if (section == SEC_DEFINITIONS) {
                section = SEC_RULES;
                continue;
            } else if (section == SEC_RULES) {
                // Process any pending rule
                if (!currentRule.empty() && currentLHS >= 0) {
                    processRule(currentRule, currentLHS);
                    currentRule.clear();
                }
                section = SEC_USER_CODE;
                continue;
            }
        }

        if (section == SEC_DEFINITIONS) {
            if (trimmed.empty() || trimmed[0] == '#') continue;

            if (trimmed.find("%token") == 0 || trimmed.find("%left") == 0 ||
                trimmed.find("%right") == 0 || trimmed.find("%nonassoc") == 0) {

                GrammarSymbol::Assoc assoc = GrammarSymbol::NONE;
                if (trimmed.find("%left") == 0) { assoc = GrammarSymbol::LEFT; currentPrecedence++; }
                else if (trimmed.find("%right") == 0) { assoc = GrammarSymbol::RIGHT; currentPrecedence++; }
                else if (trimmed.find("%nonassoc") == 0) { currentPrecedence++; }

                std::istringstream iss(trimmed);
                std::string directive;
                iss >> directive;
                std::string token;
                bool inComment = false;
                while (iss >> token) {
                    if (inComment) {
                        if (token.find("*/") != std::string::npos) inComment = false;
                        continue;
                    }
                    if (token.find("/*") == 0) {
                        if (token.find("*/") == std::string::npos) inComment = true;
                        continue;
                    }
                    if (token.back() == ',') token.pop_back();
                    if (token.empty()) continue;
                    int sid = getOrAddSymbol(token, true);
                    spec.symbols[sid].assoc = assoc;
                    spec.symbols[sid].precedence = currentPrecedence;
                    if (std::find(spec.tokenNames.begin(), spec.tokenNames.end(), token) == spec.tokenNames.end()) {
                        spec.tokenNames.push_back(token);
                    }
                }
                continue;
            }

            if (trimmed.find("%type") == 0) {
                std::istringstream iss(trimmed);
                std::string d, tag;
                iss >> d >> tag;
                // Strip angle brackets: <tlist> → tlist
                if (tag.size() >= 2 && tag.front() == '<' && tag.back() == '>') {
                    tag = tag.substr(1, tag.size() - 2);
                }
                std::string sym;
                while (iss >> sym) {
                    if (sym.back() == ',') sym.pop_back();
                    if (!sym.empty()) spec.typeTags[sym] = tag;
                }
                continue;
            }

            if (trimmed.find("%union") == 0) {
                // Multi-line union: accumulate lines until matching }
                std::string unionBody;
                size_t pos = trimmed.find('{');
                if (pos != std::string::npos) {
                    unionBody = trimmed.substr(pos + 1) + "\n";
                    int depth = 1;
                    while (depth > 0 && std::getline(file, line)) {
                        for (char c : line) {
                            if (c == '{') depth++;
                            else if (c == '}') {
                                depth--;
                                if (depth == 0) break;
                            }
                        }
                        if (depth > 0) {
                            unionBody += line + "\n";
                        } else {
                            // Include part before the closing }
                            size_t closePos = line.find('}');
                            if (closePos != std::string::npos) {
                                unionBody += line.substr(0, closePos);
                            }
                        }
                    }
                    spec.unionCode = unionBody;
                }
                continue;
            }

            if (trimmed.find("%start") == 0) {
                std::istringstream iss(trimmed);
                std::string d, s;
                iss >> d >> s;
                if (!s.empty()) spec.startSymbol = s;
                continue;
            }
        }

        if (section == SEC_RULES) {
            if (trimmed.empty()) continue;

            // Check if this line starts a new rule (has ':' before any '{')
            bool hasColon = false;
            int colonPos = -1;
            for (size_t i = 0; i < trimmed.size(); i++) {
                if (trimmed[i] == '{') break;
                if (trimmed[i] == ':') { hasColon = true; colonPos = (int)i; break; }
            }

            if (hasColon) {
                // Process previous rule
                if (!currentRule.empty() && currentLHS >= 0) {
                    processRule(currentRule, currentLHS);
                }
                currentLHS = getOrAddSymbol(trim(trimmed.substr(0, colonPos)), false);
                currentRule = trimmed.substr(colonPos + 1);
            } else {
                // Continuation: add to current rule
                if (!currentRule.empty()) currentRule += " ";
                currentRule += trimmed;
            }
        }

        if (section == SEC_USER_CODE) {
            spec.userFunctions += line + "\n";
        }
    }

    // Process final pending rule
    if (!currentRule.empty() && currentLHS >= 0) {
        processRule(currentRule, currentLHS);
    }

    if (!spec.startSymbol.empty()) {
        // already set
    } else if (!spec.productions.empty()) {
        spec.startSymbol = spec.symbols[spec.productions[0].lhs].name;
    }

    std::cout << "Grammar parsed: " << spec.symbols.size() << " symbols, "
              << spec.productions.size() << " productions" << std::endl;

    return spec;
}

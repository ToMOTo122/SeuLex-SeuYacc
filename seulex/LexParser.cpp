#include "LexParser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cctype>

std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r'))
        start++;
    size_t end = s.size();
    while (end > start && (s[end-1] == ' ' || s[end-1] == '\t' || s[end-1] == '\r' || s[end-1] == '\n'))
        end--;
    return s.substr(start, end - start);
}

int findActionBrace(const std::string& line) {
    int braceDepth = 0;
    bool inQuote = false;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '"') inQuote = !inQuote;
        if (inQuote) continue;
        if (line[i] == '{') {
            if (braceDepth == 0 && i > 0 && (line[i-1] == ' ' || line[i-1] == '\t'))
                return static_cast<int>(i);
            braceDepth++;
        } else if (line[i] == '}') {
            braceDepth--;
        }
    }
    return -1;
}

// Split a non-braced rule line into regex and action
// Pattern starts at column 0, action follows after whitespace
static void splitRuleLine(const std::string& trimmed, std::string& regex, std::string& action) {
    regex.clear();
    action.clear();
    if (trimmed.empty()) return;

    size_t i = 0;
    // Scan through the pattern
    bool inQuote = false;
    bool inBracket = false;
    while (i < trimmed.size()) {
        char c = trimmed[i];
        if (c == '"' && !inBracket) inQuote = !inQuote;
        if (c == '[' && !inQuote) inBracket = true;
        if (c == ']' && !inQuote) inBracket = false;
        if (!inQuote && !inBracket && (c == ' ' || c == '\t')) break;
        i++;
    }

    regex = trim(trimmed.substr(0, i));
    // Skip whitespace between pattern and action
    while (i < trimmed.size() && (trimmed[i] == ' ' || trimmed[i] == '\t')) i++;
    action = trimmed.substr(i);
}

LexSpec parseLexFile(const std::string& filepath) {
    LexSpec spec;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << filepath << std::endl;
        return spec;
    }

    ParseState state = ParseState::STATE_DEFINE;
    bool inUserBlock = false;
    bool inAction = false;
    std::string currentRegex, currentAction;
    int braceDepth = 0;
    std::string line;

    while (std::getline(file, line)) {
        std::string trimmed = trim(line);

        // Handle %{ ... %} in definition area
        if (state == ParseState::STATE_DEFINE) {
            if (trimmed == "%{") {
                inUserBlock = true;
                continue;
            }
            if (trimmed == "%}") {
                inUserBlock = false;
                continue;
            }
        }

        if (inUserBlock) {
            spec.user_code += line + "\n";
            continue;
        }

        // State switching on %%
        if (trimmed == "%%") {
            if (state == ParseState::STATE_DEFINE) {
                state = ParseState::STATE_RULES;
            } else if (state == ParseState::STATE_RULES) {
                state = ParseState::STATE_USER_CODE;
            }
            continue;
        }

        // ========== Definition area ==========
        if (state == ParseState::STATE_DEFINE) {
            if (trimmed.empty() || trimmed[0] == '#')
                continue;

            size_t spacePos = std::string::npos;
            for (size_t i = 0; i < trimmed.size(); i++) {
                if (trimmed[i] == ' ' || trimmed[i] == '\t') {
                    spacePos = i;
                    break;
                }
            }
            if (spacePos != std::string::npos) {
                std::string name = trim(trimmed.substr(0, spacePos));
                std::string regex = trim(trimmed.substr(spacePos + 1));
                if (!name.empty() && !regex.empty()) {
                    spec.defs[name] = regex;
                }
            }
        }

        // ========== Rules area ==========
        else if (state == ParseState::STATE_RULES) {
            if (trimmed.empty()) continue;

            if (!inAction) {
                int bracePos = findActionBrace(trimmed);
                if (bracePos >= 0) {
                    // Braced action: pattern { ... }
                    currentRegex = trim(trimmed.substr(0, bracePos));
                    if (currentRegex.empty()) continue;
                    inAction = true;
                    braceDepth = 1;
                    std::string afterBrace = trimmed.substr(bracePos + 1);

                    size_t pos;
                    for (pos = 0; pos < afterBrace.size(); pos++) {
                        if (afterBrace[pos] == '{') braceDepth++;
                        if (afterBrace[pos] == '}') {
                            braceDepth--;
                            if (braceDepth == 0) break;
                        }
                    }
                    if (braceDepth == 0) {
                        currentAction = afterBrace.substr(0, pos);
                        spec.rules.push_back({currentRegex, currentAction});
                        inAction = false;
                    } else {
                        currentAction = afterBrace;
                        currentAction += "\n";
                    }
                } else {
                    // Non-braced action: pattern action;
                    std::string regex, action;
                    splitRuleLine(trimmed, regex, action);
                    if (!regex.empty()) {
                        // Remove trailing comment from action
                        size_t cmt = action.find("/*");
                        if (cmt != std::string::npos) {
                            action = trim(action.substr(0, cmt));
                        }
                        spec.rules.push_back({regex, action});
                    }
                }
            } else {
                // Continuing multi-line braced action
                size_t pos;
                bool closed = false;
                for (pos = 0; pos < line.size(); pos++) {
                    if (line[pos] == '{') braceDepth++;
                    if (line[pos] == '}') {
                        braceDepth--;
                        if (braceDepth == 0) { closed = true; break; }
                    }
                }
                if (closed) {
                    currentAction += line.substr(0, pos);
                    spec.rules.push_back({currentRegex, currentAction});
                    inAction = false;
                } else {
                    currentAction += line + "\n";
                }
            }
        }

        // ========== User code area ==========
        else {
            spec.user_functions += line + "\n";
        }
    }

    std::cout << "Parsed: " << spec.defs.size() << " defs, "
              << spec.rules.size() << " rules, "
              << spec.user_code.size() << " bytes user code, "
              << spec.user_functions.size() << " bytes user funcs" << std::endl;

    return spec;
}

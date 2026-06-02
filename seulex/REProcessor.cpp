#include "REProcessor.h"
#include <stack>
#include <iostream>
#include <cctype>
#include <set>

std::unordered_map<char, std::set<char>> charClassTable;
// Use 0x80-0xFE range for class markers to avoid colliding with
// ASCII printable chars AND control/whitespace chars (\t=9, \n=10, \r=13, etc.)
static unsigned char nextClassMarker = 0x80;

char registerCharClass(const std::set<char>& chars) {
    if (nextClassMarker > 0xFE) {
        // Overflow: too many char classes — should not happen for typical grammars
        return (char)0x80;
    }
    char marker = (char)nextClassMarker++;
    charClassTable[marker] = chars;
    return marker;
}

static char resolveEscape(char c) {
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '0': return '\0';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"': return '"';
        default: return c;
    }
}

static bool isMetaChar(char c) {
    return c == '(' || c == ')' || c == '|' || c == '*' || c == '+' || c == '?' || c == '.';
}

// Convert a literal char to its RE representation
// Meta-characters get backslash-escaped
static std::string literalChar(char c) {
    if (isMetaChar(c) || c == '\\') {
        return std::string("\\") + c;
    }
    return std::string(1, c);
}

static std::string expandCharClass(const std::string& inner, bool complement) {
    std::set<char> chars;
    for (size_t i = 0; i < inner.size(); i++) {
        if (inner[i] == '\\' && i + 1 < inner.size()) {
            chars.insert(resolveEscape(inner[++i]));
        } else if (inner[i] == '-' && i > 0 && i + 1 < inner.size()) {
            unsigned char start = (unsigned char)inner[i - 1];
            unsigned char end = (unsigned char)inner[i + 1];
            for (unsigned char ch = start + 1; ch <= end; ch++) {
                chars.insert((char)ch);
            }
            i++;
        } else {
            chars.insert(inner[i]);
        }
    }

    if (complement) {
        std::set<char> comp;
        for (int c = 0; c < 128; c++) {
            if (chars.find((char)c) == chars.end()) {
                comp.insert((char)c);
            }
        }
        chars = comp;
    }

    if (chars.size() <= 6) {
        std::string result;
        bool first = true;
        for (char c : chars) {
            if (!first) result += '|';
            result += literalChar(c);
            first = false;
        }
        if (result.size() <= 2 && !isMetaChar(result[0])) return result;
        return "(" + result + ")";
    }

    char marker = registerCharClass(chars);
    return std::string(1, marker);
}

std::string extractLastOperand(const std::string& s) {
    if (s.empty()) return "";
    // Handle escaped char: \c
    if (s.back() != ')' && s.size() >= 2 && s[s.size()-2] == '\\') {
        return s.substr(s.size() - 2);
    }
    if (s.back() == ')') {
        int depth = 0;
        size_t pos = s.size() - 1;
        do {
            if (s[pos] == ')') depth++;
            if (s[pos] == '(') depth--;
            if (pos == 0) break;
            pos--;
        } while (depth > 0 && pos > 0);
        if (depth == 0) pos++;
        return s.substr(pos);
    }
    return std::string(1, s.back());
}

std::string expandDefs(const std::string& regex,
                       const std::unordered_map<std::string, std::string>& defs) {
    std::string result = regex;
    bool changed = true;
    int maxIter = 20;
    while (changed && maxIter-- > 0) {
        changed = false;
        for (const auto& [name, def] : defs) {
            std::string pattern = "{" + name + "}";
            size_t pos = 0;
            while ((pos = result.find(pattern, pos)) != std::string::npos) {
                result.replace(pos, pattern.size(), "(" + def + ")");
                changed = true;
            }
        }
    }
    return result;
}

std::string normalizeRE(const std::string& re) {
    std::string result;

    for (size_t i = 0; i < re.size(); i++) {
        char c = re[i];

        if (c == '\\' && i + 1 < re.size()) {
            result += resolveEscape(re[++i]);
            continue;
        }

        if (c == '[') {
            size_t j = i + 1;
            bool complement = false;
            if (j < re.size() && re[j] == '^') { complement = true; j++; }

            std::string inner;
            while (j < re.size() && re[j] != ']') {
                if (re[j] == '\\' && j + 1 < re.size()) {
                    inner += re[j];
                    inner += re[++j];
                } else {
                    inner += re[j];
                }
                j++;
            }
            i = j;
            result += expandCharClass(inner, complement);
            continue;
        }

        if (c == '"') {
            size_t j = i + 1;
            std::string literal;
            while (j < re.size() && re[j] != '"') {
                if (re[j] == '\\' && j + 1 < re.size()) {
                    literal += resolveEscape(re[++j]);
                } else {
                    literal += re[j];
                }
                j++;
            }
            i = j;
            if (literal.size() == 1) {
                // Single literal char: escape meta-chars
                result += literalChar(literal[0]);
            } else {
                result += '(';
                for (size_t k = 0; k < literal.size(); k++) {
                    if (k > 0) result += '.';
                    result += literalChar(literal[k]);
                }
                result += ')';
            }
            continue;
        }

        if (c == '.') {
            // Wildcard: use a class marker for all non-newline chars
            std::set<char> dotChars;
            for (int ch = 0; ch < 256; ch++) {
                if (ch != '\n') dotChars.insert((char)ch);
            }
            char marker = registerCharClass(dotChars);
            result += marker;
            continue;
        }

        if (c == '?') {
            // Leave '?' as a postfix operator; NFABuilder handles it via Thompson:
            //   q? = new_start --ε--> q_start, q_end --ε--> new_end
            //        new_start --ε--> new_end  (the "empty" path)
            result += c;
            continue;
        }

        if (c == '+') {
            std::string prev = extractLastOperand(result);
            result = result.substr(0, result.size() - prev.size());
            result += '(' + prev + '.' + prev + "*)";
            continue;
        }

        result += c;
    }

    return result;
}

static bool isOperator(char c) {
    return c == '(' || c == ')' || c == '|' || c == '.' || c == '*' || c == '?' || c == '+';
}

std::string addDots(const std::string& re) {
    // We process the RE character by character.
    // An "operand token" can be:
    //   - a single non-operator char (including class markers)
    //   - an escape pair \c (two chars treated as one operand)
    // We need to insert a '.' between two consecutive operand tokens, or
    // between ) /* ? + and an operand/( , or between an operand and (.
    //
    // Strategy: iterate, track what the "previous token type" was.
    // prevIsOperand = true after processing an operand token (non-operator, or \c, or ) * ? +)

    std::string result;
    // "last output token type":
    //   0 = nothing / operator that does NOT need concat after it (|, (, .)
    //   1 = operand or postfix-op or ) → a '.' may be needed before the next token
    int prevKind = 0; // 0=none/binary-op/(  1=operand/postfix-op/)

    for (size_t i = 0; i < re.size(); i++) {
        char c = re[i];

        if (c == '\\' && i + 1 < re.size()) {
            // Escape pair \c → single operand token
            char escaped = re[i + 1];
            // Insert concat dot if needed
            if (prevKind == 1) result += '.';
            result += c;
            result += escaped;
            i++; // consume the escaped char
            prevKind = 1;
            continue;
        }

        if (c == '(') {
            if (prevKind == 1) result += '.'; // operand/postfix before ( → concat
            result += c;
            prevKind = 0;
            continue;
        }

        if (c == ')' || c == '*' || c == '?' || c == '+') {
            // These never need a dot before them (they follow their operand directly)
            result += c;
            prevKind = 1; // after ) * ? +, a dot may be needed
            continue;
        }

        if (c == '|' || c == '.') {
            // Binary operators: no dot before or after (handled by surrounding operands)
            result += c;
            prevKind = 0;
            continue;
        }

        // Ordinary operand (regular char or class marker)
        if (prevKind == 1) result += '.';
        result += c;
        prevKind = 1;
    }

    return result;
}

int priority(char c) {
    switch (c) {
        case '*': case '?': case '+': return 3;
        case '.': return 2;
        case '|': return 1;
        default: return 0;
    }
}

std::string infixToPostfix(const std::string& re) {
    std::string output;
    std::stack<char> ops;
    ops.push('#');

    for (size_t i = 0; i < re.size(); i++) {
        char c = re[i];

        // Handle escape: \c → literal operand c
        if (c == '\\' && i + 1 < re.size()) {
            char literal = re[++i];
            output += '\\';
            output += literal;
            continue;
        }

        if (c == '(' || c == ')' || c == '|' || c == '.' || c == '*' || c == '+' || c == '?') {
            if (c == '(') {
                ops.push('(');
            } else if (c == ')') {
                while (ops.top() != '(') {
                    output += ops.top();
                    ops.pop();
                }
                ops.pop();
            } else {
                while (ops.top() != '#' && priority(ops.top()) >= priority(c)) {
                    output += ops.top();
                    ops.pop();
                }
                ops.push(c);
            }
        } else {
            output += c;
        }
    }

    while (ops.top() != '#') {
        output += ops.top();
        ops.pop();
    }
    return output;
}
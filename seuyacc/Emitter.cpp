#include "Emitter.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <algorithm>
#include <tuple>

// Collect all regex matches in a string
static std::vector<std::pair<size_t, size_t>> collectMatches(
    const std::string& s, const std::regex& re)
{
    std::vector<std::pair<size_t, size_t>> matches;
    auto begin = std::sregex_iterator(s.begin(), s.end(), re);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        matches.push_back({it->position(), it->length()});
    }
    return matches;
}

// Get the type tag for a symbol from %type declarations
static std::string getTypeTag(const GrammarSpec& spec, int symbolId) {
    const std::string& symName = spec.symbols[symbolId].name;
    auto it = spec.typeTags.find(symName);
    if (it != spec.typeTags.end()) {
        return it->second;
    }
    return ""; // no type tag, use whole union
}

// Translate Yacc pseudo-variables in action code to C code
// Uses production info to determine types of $N references
static std::string translateAction(const std::string& action,
                                    const Production& prod,
                                    const GrammarSpec& spec) {
    std::string result = action;

    // Get LHS type tag for $$
    std::string lhsTag = getTypeTag(spec, prod.lhs);

    // Get RHS type tags for $N
    std::vector<std::string> rhsTags;
    for (int sid : prod.rhs) {
        rhsTags.push_back(getTypeTag(spec, sid));
    }

    // Collect all $<tag>N matches and replace right-to-left
    {
        std::regex re(R"(\$<(\w+)>(\d+))");
        auto begin = std::sregex_iterator(result.begin(), result.end(), re);
        auto end = std::sregex_iterator();
        std::vector<std::tuple<size_t, size_t, std::string>> reps;
        for (auto it = begin; it != end; ++it) {
            int idx = std::stoi((*it)[2].str()) - 1;
            std::string r = "yy_v[" + std::to_string(idx) + "]." + (*it)[1].str();
            reps.push_back({it->position(), it->length(), r});
        }
        std::sort(reps.begin(), reps.end(), std::greater<>());
        for (auto& [pos, len, r] : reps) {
            result.replace(pos, len, r);
        }
    }

    // Collect all $<tag>$ matches
    {
        std::regex re(R"(\$<(\w+)>\$)");
        auto begin = std::sregex_iterator(result.begin(), result.end(), re);
        auto end = std::sregex_iterator();
        std::vector<std::tuple<size_t, size_t, std::string>> reps;
        for (auto it = begin; it != end; ++it) {
            std::string r = "yy_result." + (*it)[1].str();
            reps.push_back({it->position(), it->length(), r});
        }
        std::sort(reps.begin(), reps.end(), std::greater<>());
        for (auto& [pos, len, r] : reps) {
            result.replace(pos, len, r);
        }
    }

    // Collect all $N matches (plain references without type tag)
    {
        std::regex re(R"(\$(\d+))");
        auto begin = std::sregex_iterator(result.begin(), result.end(), re);
        auto end = std::sregex_iterator();
        std::vector<std::tuple<size_t, size_t, std::string>> reps;
        for (auto it = begin; it != end; ++it) {
            int idx = std::stoi((*it)[1].str()) - 1;
            std::string r = "yy_v[" + std::to_string(idx) + "]";
            if (idx >= 0 && idx < (int)rhsTags.size() && !rhsTags[idx].empty()) {
                r += "." + rhsTags[idx];
            }
            reps.push_back({it->position(), it->length(), r});
        }
        std::sort(reps.begin(), reps.end(), std::greater<>());
        for (auto& [pos, len, r] : reps) {
            result.replace(pos, len, r);
        }
    }

    // Replace remaining standalone $$
    {
        size_t pos = 0;
        std::string replacement = "yy_result";
        if (!lhsTag.empty()) replacement += "." + lhsTag;
        while ((pos = result.find("$$", pos)) != std::string::npos) {
            result.replace(pos, 2, replacement);
            pos += replacement.size();
        }
    }

    return result;
}

void generateYaccCode(const ParseTables& tables, const std::string& outputPath) {
    std::ofstream out(outputPath);
    if (!out.is_open()) {
        std::cerr << "Error: cannot create " << outputPath << std::endl;
        return;
    }

    // ====== 1. User code from %{...%} ======
    if (!tables.grammar.userCode.empty()) {
        out << tables.grammar.userCode << "\n";
    } else {
        out << "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n";
    }

    // ====== 2. Token definitions ======
    out << "/* Token definitions */\n";
    int tokenCode = 257;
    for (const auto& tokenName : tables.grammar.tokenNames) {
        auto it = tables.symbolMap.find(tokenName);
        if (it != tables.symbolMap.end()) {
            out << "#define " << tokenName << " " << tokenCode << "\n";
            tokenCode++;
        }
    }
    out << "\n";

    // ====== 3. Declarations ======
    out << R"(/* Parser tables */
#define YY_NSTATES )" << tables.action.size() << R"(
#define YY_NPRODS  )" << tables.productions.size() << R"(

extern int yylex(void);
extern char* yytext;
extern int yylineno;

int yyparse(void);
void yyerror(const char* s);

)";

    // Union type
    if (!tables.grammar.unionCode.empty()) {
        out << "typedef union {\n" << tables.grammar.unionCode << "\n} YYSTYPE;\n\n";
    } else {
        out << "typedef struct { int type; int value; char* name; } YYSTYPE;\n\n";
    }

    out << "YYSTYPE yylval;\n\n";

    // ====== 4. Production table ======
    out << "static int yy_prod_lhs[" << tables.productions.size() << "] = {";
    for (size_t i = 0; i < tables.productions.size(); i++) {
        if (i > 0) out << ", ";
        out << tables.productions[i].lhs;
    }
    out << "};\n\n";

    out << "static int yy_prod_len[" << tables.productions.size() << "] = {";
    for (size_t i = 0; i < tables.productions.size(); i++) {
        if (i > 0) out << ", ";
        out << tables.productions[i].rhs.size();
    }
    out << "};\n\n";

    // ====== 5. ACTION table ======
    out << R"(/* ACTION table */
#define YY_SHIFT  1
#define YY_REDUCE 2
#define YY_ACCEPT 3
#define YY_ERROR  0

typedef struct { int type; int value; } YYAction;

)";

    int nStates = static_cast<int>(tables.action.size());
    int maxTerm = 0;
    for (const auto& sym : tables.symbols) {
        if (sym.isTerminal && sym.id > maxTerm) maxTerm = sym.id;
    }

    out << "static YYAction yy_action[" << nStates << "][" << (maxTerm + 1) << "] = {\n";
    for (int s = 0; s < nStates; s++) {
        out << "    {";
        for (int t = 0; t <= maxTerm; t++) {
            if (t > 0) out << ",";
            int act_type = 0, act_value = 0;
            auto it = tables.action[s].find(t);
            if (it != tables.action[s].end()) {
                act_type = (it->second.type == SHIFT) ? 1 :
                          (it->second.type == REDUCE) ? 2 :
                          (it->second.type == ACCEPT) ? 3 : 0;
                act_value = it->second.value;
            }
            out << "{" << act_type << "," << act_value << "}";
        }
        out << "}";
        if (s < nStates - 1) out << ",";
        out << "\n";
    }
    out << "};\n\n";

    // ====== 6. GOTO table ======
    out << "static int yy_goto[" << nStates << "][" << (int)tables.symbols.size() << "] = {\n";
    for (int s = 0; s < nStates; s++) {
        out << "    {";
        for (size_t nt = 0; nt < tables.symbols.size(); nt++) {
            if (nt > 0) out << ",";
            int target = -1;
            auto it = tables.gotoTable[s].find((int)nt);
            if (it != tables.gotoTable[s].end()) target = it->second;
            out << target;
        }
        out << "}";
        if (s < nStates - 1) out << ",";
        out << "\n";
    }
    out << "};\n\n";

    // ====== 7. yyparse() and action execution ======
    out << R"(/* Semantic value stacks */
static YYSTYPE yy_val_stack[1024];
static int yy_val_top = 0;

static void yy_push_val(YYSTYPE v) { yy_val_stack[yy_val_top++] = v; }
static YYSTYPE yy_pop_val(void) { return yy_val_stack[--yy_val_top]; }

/* State stack */
static int yy_state_stack[1024];
static int yy_state_top = 0;

static void yy_execute_action(int prod_id) {
    int len = yy_prod_len[prod_id];
    YYSTYPE yy_result;
    YYSTYPE yy_v[32]; /* RHS values */

    /* Pop RHS values from stack (in reverse order) */
    for (int i = len - 1; i >= 0; i--) {
        yy_v[i] = yy_pop_val();
    }

    switch (prod_id) {
)";

    // Generate action cases with translated pseudo-variables
    for (const auto& prod : tables.productions) {
        out << "        case " << prod.id << ": ";
        if (!prod.actionCode.empty()) {
            std::string translated = translateAction(prod.actionCode, prod, tables.grammar);
            out << "{\n";
            out << "            " << translated << "\n";
            out << "        }\n";
        } else {
            out << "/* no action */\n";
        }
        out << "        break;\n";
    }

    out << R"(        default: break;
    }

    /* Push result */
    yy_push_val(yy_result);
}

int yyparse(void) {
    yy_state_top = 0;
    yy_state_stack[yy_state_top++] = 0;
    yy_val_top = 0;

    int token = yylex();

    while (1) {
        int s = yy_state_stack[yy_state_top - 1];
        int t = token;

        if (t >= )" << (maxTerm + 1) << R"() t = 0;

        YYAction act = yy_action[s][t];

        if (act.type == YY_SHIFT) {
            yy_state_stack[yy_state_top++] = act.value;
            yy_push_val(yylval);
            token = yylex();
        } else if (act.type == YY_REDUCE) {
            int prod_id = act.value;
            int lhs = yy_prod_lhs[prod_id];

            yy_execute_action(prod_id);

            /* Pop |rhs| states */
            yy_state_top -= yy_prod_len[prod_id];
            int new_s = yy_state_stack[yy_state_top - 1];
            int next = yy_goto[new_s][lhs];
            if (next < 0) {
                yyerror("syntax error in goto");
                return 1;
            }
            yy_state_stack[yy_state_top++] = next;
        } else if (act.type == YY_ACCEPT) {
            return 0;
        } else {
            yyerror("syntax error");
            token = yylex();
            if (token == 0) return 1;
        }
    }
}
)";

    // ====== 8. User functions ======
    if (!tables.grammar.userFunctions.empty()) {
        out << "\n" << tables.grammar.userFunctions;
    }

    out.close();
    std::cout << "Generated " << outputPath << std::endl;
}

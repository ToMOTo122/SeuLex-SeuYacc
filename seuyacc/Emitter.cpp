#include "Emitter.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <algorithm>
#include <tuple>

// Get the union type tag for a symbol (from %type declarations)
static std::string getTypeTag(const GrammarSpec& spec, int symbolId) {
    const std::string& symName = spec.symbols[symbolId].name;
    auto it = spec.typeTags.find(symName);
    if (it != spec.typeTags.end()) return it->second;
    return "";
}

// Translate Yacc pseudo-variables ($$, $N, $<tag>N) to C code
static std::string translateAction(const std::string& action,
                                    const Production& prod,
                                    const GrammarSpec& spec) {
    std::string result = action;
    std::string lhsTag = getTypeTag(spec, prod.lhs);
    std::vector<std::string> rhsTags;
    for (int sid : prod.rhs)
        rhsTags.push_back(getTypeTag(spec, sid));

    // Replace $<tag>N
    {
        std::regex re(R"(\$<(\w+)>(\d+))");
        auto begin = std::sregex_iterator(result.begin(), result.end(), re);
        auto end   = std::sregex_iterator();
        std::vector<std::tuple<size_t,size_t,std::string>> reps;
        for (auto it = begin; it != end; ++it) {
            int idx = std::stoi((*it)[2].str()) - 1;
            std::string r = "yy_v[" + std::to_string(idx) + "]." + (*it)[1].str();
            reps.push_back(std::make_tuple(it->position(), it->length(), r));
        }
        std::sort(reps.begin(), reps.end(), std::greater<std::tuple<size_t,size_t,std::string>>());
        for (size_t i = 0; i < reps.size(); i++)
            result.replace(std::get<0>(reps[i]), std::get<1>(reps[i]), std::get<2>(reps[i]));
    }
    // Replace $<tag>$
    {
        std::regex re(R"(\$<(\w+)>\$)");
        auto begin = std::sregex_iterator(result.begin(), result.end(), re);
        auto end   = std::sregex_iterator();
        std::vector<std::tuple<size_t,size_t,std::string>> reps;
        for (auto it = begin; it != end; ++it) {
            std::string r = "yy_result." + (*it)[1].str();
            reps.push_back(std::make_tuple(it->position(), it->length(), r));
        }
        std::sort(reps.begin(), reps.end(), std::greater<std::tuple<size_t,size_t,std::string>>());
        for (size_t i = 0; i < reps.size(); i++)
            result.replace(std::get<0>(reps[i]), std::get<1>(reps[i]), std::get<2>(reps[i]));
    }
    // Replace $N
    {
        std::regex re(R"(\$(\d+))");
        auto begin = std::sregex_iterator(result.begin(), result.end(), re);
        auto end   = std::sregex_iterator();
        std::vector<std::tuple<size_t,size_t,std::string>> reps;
        for (auto it = begin; it != end; ++it) {
            int idx = std::stoi((*it)[1].str()) - 1;
            std::string r = "yy_v[" + std::to_string(idx) + "]";
            if (idx >= 0 && idx < (int)rhsTags.size() && !rhsTags[idx].empty())
                r += "." + rhsTags[idx];
            reps.push_back(std::make_tuple(it->position(), it->length(), r));
        }
        std::sort(reps.begin(), reps.end(), std::greater<std::tuple<size_t,size_t,std::string>>());
        for (size_t i = 0; i < reps.size(); i++)
            result.replace(std::get<0>(reps[i]), std::get<1>(reps[i]), std::get<2>(reps[i]));
    }
    // Replace $$
    {
        size_t pos = 0;
        std::string repl = "yy_result";
        if (!lhsTag.empty()) repl += "." + lhsTag;
        while ((pos = result.find("$$", pos)) != std::string::npos) {
            result.replace(pos, 2, repl);
            pos += repl.size();
        }
    }
    return result;
}

// Remove the standalone main() from minic.y's user_functions section.
static std::string stripMainFromUserFunctions(const std::string& src) {
    std::string result = src;
    size_t pos = 0;
    while ((pos = result.find("main(", pos)) != std::string::npos) {
        size_t funcStart = pos;
        while (funcStart > 0 &&
               (result[funcStart-1]==' ' || result[funcStart-1]=='\t' ||
                result[funcStart-1]=='\n'|| result[funcStart-1]=='\r'))
            funcStart--;
        while (funcStart > 0 &&
               result[funcStart-1]!='\n' && result[funcStart-1]!='\r')
            funcStart--;
        size_t bs = result.find('{', pos);
        if (bs == std::string::npos) break;
        int depth = 0;
        size_t be = bs;
        for (; be < result.size(); be++) {
            if (result[be]=='{') depth++;
            else if (result[be]=='}') { depth--; if (depth==0) break; }
        }
        result.replace(funcStart, be - funcStart + 1,
                       "/* main() from .y removed: see minic_driver.c */");
        pos = funcStart + 1;
    }
    return result;
}

// Main entry point: generate yyparse.c and minic.tab.h from parse tables
void generateYaccCode(const ParseTables& tables, const std::string& outputPath) {
    std::ofstream out(outputPath);
    if (!out.is_open()) {
        std::cerr << "Error: cannot create " << outputPath << std::endl;
        return;
    }

    // NEW: Generate minic.tab.h so the Lexer stays synced with the Parser
    std::ofstream tabOut("minic.tab.h");
    if (!tabOut.is_open()) {
        std::cerr << "Warning: cannot create minic.tab.h" << std::endl;
    } else {
        tabOut << "/* Auto-generated by SeuYacc */\n";
        tabOut << "#ifndef MINIC_TAB_H\n#define MINIC_TAB_H\n\n";
        tabOut << "#include \"symtab.h\"\n#include \"types.h\"\n\n";
    }

    // === 1. User code from %{...%} ===
    if (!tables.grammar.userCode.empty()) {
        out << tables.grammar.userCode << "\n";
    } else {
        out << "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n";
    }

    // === 2. Token #defines (INT=257, FLOAT=258, ...) ===
    out << "/* Token definitions */\n";
    int tokenCode = 257;
    std::vector<std::string> tokNames;
    std::vector<int>         tokVals;
    std::vector<int>         tokSymIds;

    for (size_t ti = 0; ti < tables.grammar.tokenNames.size(); ti++) {
        const std::string& tokenName = tables.grammar.tokenNames[ti];
        auto it = tables.symbolMap.find(tokenName);
        if (it != tables.symbolMap.end()) {
            out << "#define " << tokenName << " " << tokenCode << "\n";
            // Write to header file too
            if (tabOut.is_open()) {
                tabOut << "#define " << tokenName << " " << tokenCode << "\n";
            }
            tokNames.push_back(tokenName);
            tokVals.push_back(tokenCode);
            tokSymIds.push_back(it->second);
            tokenCode++;
        }
    }
    out << "\n";

    // Close out the header file cleanly
    if (tabOut.is_open()) {
        if (!tables.grammar.unionCode.empty()) {
            tabOut << "\ntypedef union {\n" << tables.grammar.unionCode << "\n} YYSTYPE;\n\n";
        } else {
            tabOut << "\ntypedef struct { int type; int value; char* name; } YYSTYPE;\n\n";
        }
        tabOut << "extern YYSTYPE yylval;\n\n";
        tabOut << "#endif /* MINIC_TAB_H */\n";
        tabOut.close();
        std::cout << "Generated minic.tab.h" << std::endl;
    }

    // === 3. Forward declarations ===
    int nStates = static_cast<int>(tables.action.size());
    int maxTerm = 0;
    for (size_t si = 0; si < tables.symbols.size(); si++)
        if (tables.symbols[si].isTerminal && tables.symbols[si].id > maxTerm)
            maxTerm = tables.symbols[si].id;

    out << "/* Parser tables: " << nStates << " states, "
        << tables.productions.size() << " productions */\n\n";

    // Only emit declarations not already provided by the userCode block
    bool hasUserCode = !tables.grammar.userCode.empty();
    auto inUserCode = [&](const std::string& needle) {
        return hasUserCode && tables.grammar.userCode.find(needle) != std::string::npos;
    };

    if (!inUserCode("yylex")) out << "extern int yylex(void);\n";
    if (!inUserCode("yytext")) out << "extern char* yytext;\n";
    if (!inUserCode("yylineno")) out << "extern int yylineno;\n";
    if (!inUserCode("lineno")) out << "extern int lineno;\n";
    if (!inUserCode("yyin")) out << "extern FILE* yyin;\n";
    out << "\n";
    if (!inUserCode("yyparse")) out << "int yyparse(void);\n";
    if (!inUserCode("yyerror")) out << "void yyerror(const char* s);\n";
    out << "\n";

    if (!inUserCode("YYSTYPE")) {
        if (!tables.grammar.unionCode.empty()) {
            out << "typedef union {\n" << tables.grammar.unionCode << "\n} YYSTYPE;\n\n";
        } else {
            out << "typedef struct { int type; int value; char* name; } YYSTYPE;\n\n";
        }
    }
    if (!inUserCode("yylval")) out << "YYSTYPE yylval;\n";
    out << "\n";

    // === 4. Production tables ===
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

    out << "#ifdef YY_TRACE\n";
    out << "static const char* yy_prod_name[" << tables.productions.size() << "] = {\n";
    for (size_t i = 0; i < tables.productions.size(); i++) {
        std::string lhs = tables.symbols[tables.productions[i].lhs].name;
        out << "    \"" << lhs << " ->";
        for (size_t j = 0; j < tables.productions[i].rhs.size(); j++)
            out << " " << tables.symbols[tables.productions[i].rhs[j]].name;
        out << "\"";
        if (i + 1 < tables.productions.size()) out << ",";
        out << "\n";
    }
    out << "};\n#endif\n\n";

    // === 5. Token value -> symbol ID mapping ===
    out << "/* Map yylex() token values (257+) to ACTION table symbol IDs */\n";
    out << "static int yy_token_to_sym(int tok) {\n";
    out << "    switch (tok) {\n";
    {
        auto it = tables.symbolMap.find("$");
        int eofId = (it != tables.symbolMap.end()) ? it->second : 0;
        out << "        case 0:   return " << eofId << "; /* EOF */\n";
    }
    for (size_t i = 0; i < tokNames.size(); i++) {
        out << "        case " << tokVals[i] << ": return " << tokSymIds[i]
            << "; /* " << tokNames[i] << " */\n";
    }
    out << "        default:  return -1;\n";
    out << "    }\n}\n\n";

    // === 6. ACTION table ===
    out << "#define YY_SHIFT  1\n#define YY_REDUCE 2\n";
    out << "#define YY_ACCEPT 3\n#define YY_ERROR  0\n\n";
    out << "typedef struct { int type; int value; } YYAction;\n\n";
    out << "static YYAction yy_action[" << nStates << "]["
        << (maxTerm + 1) << "] = {\n";
    for (int s = 0; s < nStates; s++) {
        out << "    {";
        for (int t = 0; t <= maxTerm; t++) {
            if (t > 0) out << ",";
            int at = 0, av = 0;
            auto it = tables.action[s].find(t);
            if (it != tables.action[s].end()) {
                at = (it->second.type == SHIFT)  ? 1 :
                     (it->second.type == REDUCE) ? 2 :
                     (it->second.type == ACCEPT) ? 3 : 0;
                av = it->second.value;
            }
            out << "{" << at << "," << av << "}";
        }
        out << "}";
        if (s + 1 < nStates) out << ",";
        out << "\n";
    }
    out << "};\n\n";

    // === 7. GOTO table ===
    out << "static int yy_goto[" << nStates << "]["
        << (int)tables.symbols.size() << "] = {\n";
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
        if (s + 1 < nStates) out << ",";
        out << "\n";
    }
    out << "};\n\n";

    // === 8. Semantic stack + yy_execute_action() ===
    out << "static YYSTYPE yy_val_stack[1024];\n";
    out << "static int yy_val_top = 0;\n";
    out << "static void yy_push_val(YYSTYPE v) { yy_val_stack[yy_val_top++] = v; }\n";
    out << "static YYSTYPE yy_pop_val(void)    { return yy_val_stack[--yy_val_top]; }\n\n";
    out << "static int yy_state_stack[1024];\n";
    out << "static int yy_state_top = 0;\n\n";

    out << "static void yy_execute_action(int prod_id) {\n";
    out << "    int len = yy_prod_len[prod_id];\n";
    out << "    YYSTYPE yy_result;\n";
    out << "    yy_result.value = 0; yy_result.name = 0;\n";
    out << "    YYSTYPE yy_v[32];\n";
    out << "    int i;\n";
    out << "    for (i = len - 1; i >= 0; i--)\n";
    out << "        yy_v[i] = yy_pop_val();\n";
    out << "#ifdef YY_TRACE\n";
    out << "    printf(\"  [reduce %d] %s\\n\", prod_id, yy_prod_name[prod_id]);\n";
    out << "    fflush(stdout);\n";
    out << "#endif\n";
    out << "    switch (prod_id) {\n";

    for (size_t pi = 0; pi < tables.productions.size(); pi++) {
        const Production& prod = tables.productions[pi];
        out << "        case " << prod.id << ": ";
        if (!prod.actionCode.empty()) {
            std::string tr = translateAction(prod.actionCode, prod, tables.grammar);
            out << "{\n            " << tr << "\n        }\n";
        } else {
            out << "/* no action */\n";
        }
        out << "        break;\n";
    }
    out << "        default: break;\n";
    out << "    }\n";
    out << "    yy_push_val(yy_result);\n";
    out << "}\n\n";

    // === 9. yyparse() ===
    out << "int yyparse(void) {\n";
    out << "    yy_state_top = 0;\n";
    out << "    yy_state_stack[yy_state_top++] = 0;\n";
    out << "    yy_val_top = 0;\n";
    out << "    int token = yylex();\n";
    out << "    while (1) {\n";
    out << "        int s = yy_state_stack[yy_state_top - 1];\n";
    out << "        int t = yy_token_to_sym(token);\n";
    out << "        if (t < 0 || t > " << maxTerm << ") t = 0;\n";
    out << "        YYAction act = yy_action[s][t];\n";
    out << "        if (act.type == YY_SHIFT) {\n";
    out << "            yy_state_stack[yy_state_top++] = act.value;\n";
    out << "            yy_push_val(yylval);\n";
    out << "            token = yylex();\n";
    out << "        } else if (act.type == YY_REDUCE) {\n";
    out << "            int prod_id = act.value;\n";
    out << "            int lhs    = yy_prod_lhs[prod_id];\n";
    out << "            yy_execute_action(prod_id);\n";
    out << "            yy_state_top -= yy_prod_len[prod_id];\n";
    out << "            int new_s = yy_state_stack[yy_state_top - 1];\n";
    out << "            int next  = yy_goto[new_s][lhs];\n";
    out << "            if (next < 0) { yyerror(\"syntax error in goto\"); return 1; }\n";
    out << "            yy_state_stack[yy_state_top++] = next;\n";
    out << "        } else if (act.type == YY_ACCEPT) {\n";
    out << "            return 0;\n";
    out << "        } else {\n";
    out << "            yyerror(\"syntax error\");\n";
    out << "            token = yylex();\n";
    out << "            if (token == 0) return 1;\n";
    out << "        }\n";
    out << "    }\n}\n\n";

    // === 10. User functions ===
    if (!tables.grammar.userFunctions.empty()) {
        out << stripMainFromUserFunctions(tables.grammar.userFunctions) << "\n";
    }

    out.close();
    std::cout << "Generated " << outputPath << std::endl;
}
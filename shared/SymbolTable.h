#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

struct Symbol {
    std::string name;
    std::string type;
    int offset;
    int scopeLevel;

    Symbol() : offset(0), scopeLevel(0) {}
    Symbol(const std::string& n, const std::string& t, int o, int s)
        : name(n), type(t), offset(o), scopeLevel(s) {}
};

class SymbolTable {
    std::vector<std::unordered_map<std::string, Symbol>> scopeStack;
    int currentLevel;
    int nextOffset;

public:
    SymbolTable() : currentLevel(0), nextOffset(0) {
        scopeStack.push_back({});
    }

    void enterScope() {
        scopeStack.push_back({});
        currentLevel++;
    }

    void exitScope() {
        if (scopeStack.size() > 1) {
            scopeStack.pop_back();
            currentLevel--;
        }
    }

    bool insert(const Symbol& s) {
        auto& current = scopeStack.back();
        if (current.find(s.name) != current.end()) {
            return false;
        }
        Symbol entry = s;
        entry.scopeLevel = currentLevel;
        current[s.name] = entry;
        return true;
    }

    Symbol* lookup(const std::string& name) {
        for (int i = scopeStack.size() - 1; i >= 0; i--) {
            auto it = scopeStack[i].find(name);
            if (it != scopeStack[i].end()) {
                return &it->second;
            }
        }
        return nullptr;
    }

    bool containsInCurrentScope(const std::string& name) {
        auto& current = scopeStack.back();
        return current.find(name) != current.end();
    }

    int getCurrentLevel() const { return currentLevel; }

    int allocateOffset(int size) {
        int ret = nextOffset;
        nextOffset += size;
        return ret;
    }

    void resetOffset() {
        nextOffset = 0;
    }

    static int getTypeSize(const std::string& type) {
        if (type == "int" || type == "float") return 4;
        if (type == "char") return 1;
        if (type == "void") return 0;
        if (type.find("array") != std::string::npos) return 8;
        if (type.find("record") != std::string::npos) return 8;
        return 4;
    }
};

#endif

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <unordered_map>
#include <string>
#include "type.h"
#include "value.h"

struct NadaSymbol {
    Nada::Type    type;
    std::string   name;
    NadaValue    *value;

    NadaSymbol() : type(Nada::Undefined),value(nullptr) {}
    NadaSymbol(Nada::Type t, const std::string &n) : type(t), name(n), value(nullptr) {}
};


class NadaSymbolTable
{
public:
    NadaSymbolTable();

    bool contains(const std::string& name) const;

    bool add(const NadaSymbol &symbol);
    bool get(const std::string& name, NadaSymbol &symbol) const;

private:
    std::unordered_map<std::string, NadaSymbol> mTable;
};

#endif // SYMBOLTABLE_H

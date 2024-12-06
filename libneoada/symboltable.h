#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

#include "type.h"
#include "value.h"

struct NadaFunction {
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters; // Name und Typ der Parameter
    // const ASTNode* body; // AST-Knoten des Funktionskörpers (optional, für benutzerdefinierte Funktionen)
    std::function<NadaValue(const std::vector<NadaValue>&)> nativeImplementation; // Für eingebaute Funktionen
};

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

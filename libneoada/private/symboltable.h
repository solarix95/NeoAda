#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

#include "type.h"
#include "value.h"
#include "utils.h"

namespace Nda {

struct Symbol {
    Nda::Type        type;
    Nda::LowerString name;
    NadaValue        *value;
    Nda::LowerString typeName;

    Symbol() : type(Nda::Undefined),value(nullptr) {}
    Symbol(Nda::Type t, const std::string &n, const std::string &tn) : type(t), name(n), value(nullptr), typeName(tn) {}
};


}

class NadaSymbolTable
{
public:
    enum Scope {
        GlobalScope,
        LocalScope,         // Function-Call
        LoopScope,          // Enter While/For
        ConditionalScope    // Enter if..
    };
    NadaSymbolTable(Scope s);

    inline Scope scope() const { return mScope; }

    bool contains(const std::string& name) const;

    bool add(const Nda::Symbol &symbol);
    bool get(const std::string& name, Nda::Symbol &symbol) const;
    bool initValue(const std::string& name);

private:
    std::unordered_map<std::string, Nda::Symbol> mTable;
    Scope                                       mScope;
};

using NadaSymbolTables = std::vector<NadaSymbolTable*>;
using NadaStackFrames  = std::vector<NadaSymbolTables*>;


#endif // SYMBOLTABLE_H

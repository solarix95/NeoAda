#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

#include "type.h"
#include "variant.h"
#include "utils.h"

namespace Nda {

struct Symbol {
    Nda::LowerString        name;
    NdaVariant             *value;
    const Nda::RuntimeType *type;

    Symbol() : value(nullptr), type(nullptr) {}
    Symbol(const std::string &n, const Nda::RuntimeType *t) : name(n), value(nullptr), type(t)  {}
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

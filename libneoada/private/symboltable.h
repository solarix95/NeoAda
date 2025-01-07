#ifndef LIB_NEOADA_SYMBOLTABLE_H
#define LIB_NEOADA_SYMBOLTABLE_H

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
    ~NadaSymbolTable();

    inline Scope scope() const { return mScope; }

    bool contains(const std::string& name) const;
    int  indexOf(const std::string& name) const;

    bool add(const Nda::Symbol &symbol);
    // bool get(const std::string& name, Nda::Symbol &symbol) const;
    bool get2(const std::string& name, Nda::Symbol **symbol) const;
    void lookUp(int index, Nda::Symbol **symbol) const;
    bool initValue(const std::string& name);

private:
    struct MyHash {
        std::size_t operator()(const std::string& key) const {
            std::size_t result = 5381; // Anfangswert wählen
            for (char c : key) {
                result = (result << 5) + result + c; // Djb2-Algorithmus
            }
            return result;
        }
    };

    // std::unordered_map<std::string, Nda::Symbol*> mTable;
    std::vector<Nda::Symbol*>   mTable;
    Scope                       mScope;
};

using NadaSymbolTables = std::vector<NadaSymbolTable*>;
using NadaStackFrames  = std::vector<NadaSymbolTables*>;


#endif // SYMBOLTABLE_H

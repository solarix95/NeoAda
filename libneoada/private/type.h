#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <unordered_map>
#include "utils.h"

namespace Nda {
    enum Type { Undefined, Reference, Any, Number, Natural, Supernatural, Boolean, Byte, String, List, Dict };

    Type typeByString(const std::string &name);

    struct RuntimeType {
        LowerString name;
        Type        dataType;
        LowerString baseType;
        bool        instantiable;

        RuntimeType() : name(""), dataType(Undefined), baseType(""), instantiable(false) {}
        RuntimeType(std::string n, Type t, std::string bn, bool i) : name(n), dataType(t), baseType(bn), instantiable(i) {}
        RuntimeType(LowerString n, Type t, std::string bn, bool i) : name(n), dataType(t), baseType(bn), instantiable(i) {}
    };

    using RuntimeTypes = std::unordered_map<std::string, Nda::RuntimeType>;
}



#endif // TYPE_H

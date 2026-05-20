#include "functiontable.h"
#include "utils.h"
#include <cassert>
#include <exception>

namespace Nda {


//-------------------------------------------------------------------------------------------------
FunctionTable::FunctionTable() {}

//-------------------------------------------------------------------------------------------------
void FunctionTable::clear()
{
    mFunctions.clear();
}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::bindFnc(const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb)
{
    std::string lowerName = Nda::toLower(name);

    Nda::OverloadedFunction &variants = mFunctions[lowerName];
    variants.functionName = lowerName;

    // TODO: check if already there..
    variants.overloadsByArgCount[(int)parameters.size()].push_back(Nda::FunctionEntry{"",parameters,NdaParser::ASTNodePtr(), nullptr, std::move(cb), nullptr});

    return true;
}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::bindPrc(const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb)
{

    std::string lowerName = Nda::toLower(name);

    Nda::OverloadedFunction &variants = mFunctions[lowerName];
    variants.functionName = lowerName;

    // TODO: check if already there..
    variants.overloadsByArgCount[(int)parameters.size()].push_back(Nda::FunctionEntry{"",parameters,NdaParser::ASTNodePtr(), nullptr, nullptr, std::move(cb)});

    return true;
}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::bind(const std::string &name, const Nda::FncParameters &parameters, const NdaParser::ASTNodePtr &block)
{
    std::string lowerName = Nda::toLower(name);

    Nda::OverloadedFunction &variants = mFunctions[lowerName];
    variants.functionName = lowerName;

    // TODO: check if already there..
    variants.overloadsByArgCount[(int)parameters.size()].push_back(Nda::FunctionEntry{"",parameters,block, nullptr, nullptr, nullptr});

    return true;
}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::bind(const std::string &name, const FncParameters &parameters,Runnable *block)
{
    std::string lowerName = Nda::toLower(name);

    Nda::OverloadedFunction &variants = mFunctions[lowerName];
    variants.functionName = lowerName;

    // TODO: check if already there..
    variants.overloadsByArgCount[(int)parameters.size()].push_back(Nda::FunctionEntry{"",parameters,nullptr, block, nullptr, nullptr});

    return true;

}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::contains(const std::string &name, const NdaVariants &parameters)
{
    return symbolPtr(name, parameters) != nullptr;
}

//-------------------------------------------------------------------------------------------------
Nda::FunctionEntry *FunctionTable::symbolPtr(const std::string &name, const NdaVariants &parameters)
{
    std::string lowerName = Nda::toLower(name);
    auto functionIt = mFunctions.find(lowerName);
    if (functionIt == mFunctions.end())
        return nullptr;

    auto overloadIt = functionIt->second.overloadsByArgCount.find((int)parameters.size());
    if (overloadIt == functionIt->second.overloadsByArgCount.end())
        return nullptr;

    for (auto &variant : overloadIt->second) {
        if (matches(variant, parameters))
            return &variant;
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
Nda::FunctionEntry &FunctionTable::symbol(const std::string &name, const NdaVariants &parameters)
{
    auto *entry = symbolPtr(name, parameters);
    if (entry)
        return *entry;

    assert(false && "symbol lookup error");
    std::terminate();
}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::matches(const Nda::FunctionEntry &entry, const NdaVariants &parameters) const
{
    if (entry.parameters.size() != parameters.size())
        return false;

    for (int i=0; i<(int)parameters.size(); i++) {
        if (!parameterMatches(entry.parameters[i].type, parameters[i]))
            return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::parameterMatches(const std::string &typeName, const NdaVariant &value) const
{
    std::string lowerType = Nda::toLower(typeName);
    if (lowerType == "any")
        return true;

    if (lowerType == "number") {
        bool ok;
        value.toDouble(&ok);
        return ok;
    }

    switch (value.type()) {
    case Nda::Natural:      return lowerType == "natural";
    case Nda::Supernatural: return lowerType == "supernatural";
    case Nda::Boolean:      return lowerType == "boolean";
    case Nda::Byte:         return lowerType == "byte";
    case Nda::String:       return lowerType == "string";
    case Nda::List:         return lowerType == "list";
    case Nda::Dict:         return lowerType == "dict";
    default:                return false;
    }
}

//-------------------------------------------------------------------------------------------------
Nda::FncValues Nda::FunctionEntry::fncValues(const NdaVariants &values) const
{
    Nda::FncValues ret;
    assert(values.size() == parameters.size());
    for (int i=0; i<(int)parameters.size(); i++)
        ret[parameters[i].name] = values[i];
    return ret;
}

}

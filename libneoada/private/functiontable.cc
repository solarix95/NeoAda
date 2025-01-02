#include "functiontable.h"
#include "utils.h"
#include <cassert>
#include <exception>

namespace Nda {


//-------------------------------------------------------------------------------------------------
FunctionTable::FunctionTable() {}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::bindFnc(const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb)
{
    std::string lowerName = Nda::toLower(name);

    Nda::OverloadedFunction &variants = mFunctions[lowerName];
    variants.functionName = lowerName;

    // TODO: check if already there..
    variants.overloads.push_back(Nda::FunctionEntry{"",parameters,NadaParser::ASTNodePtr(), std::move(cb), nullptr});

    return true;
}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::bindPrc(const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb)
{

    std::string lowerName = Nda::toLower(name);

    Nda::OverloadedFunction &variants = mFunctions[lowerName];
    variants.functionName = lowerName;

    // TODO: check if already there..
    variants.overloads.push_back(Nda::FunctionEntry{"",parameters,NadaParser::ASTNodePtr(), nullptr, std::move(cb)});

    return true;
}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::bind(const std::string &name, const Nda::FncParameters &parameters, const NadaParser::ASTNodePtr &block)
{
    Nda::OverloadedFunction &variants = mFunctions[name];
    variants.functionName = name;

    // TODO: check if already there..
    variants.overloads.push_back(Nda::FunctionEntry{"",parameters,block, nullptr, nullptr});

    return true;
}

//-------------------------------------------------------------------------------------------------
bool FunctionTable::contains(const std::string &name, const NadaValues &parameters)
{
    std::string lowerName = Nda::toLower(name);
    if (mFunctions.count(lowerName) <= 0)
        return false;

    Nda::OverloadedFunction &variants = mFunctions[lowerName];
    for (const auto &variant : variants.overloads) {
        return true;
        /*
        if (variant.parameters == parameters)
            return true;
        */
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
Nda::FunctionEntry &FunctionTable::symbol(const std::string &name, const NadaValues &parameters)
{
    std::string lowerName = Nda::toLower(name);

    Nda::OverloadedFunction &variants = mFunctions[lowerName];
    for (auto &variant : variants.overloads) {

        return variant;

        // TODO: Parameters
        /*
        if (variant.parameters == parameters)
            return variant;
        */
    }

    assert(false && "symbol lookup error");
    std::terminate();
}

//-------------------------------------------------------------------------------------------------
Nda::FncValues Nda::FunctionEntry::fncValues(const NadaValues &values) const
{
    Nda::FncValues ret;
    assert(values.size() == parameters.size());
    for (int i=0; i<(int)parameters.size(); i++)
        ret[parameters[i].name] = values[i];
    return ret;
}

}

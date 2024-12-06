#include "functiontable.h"
#include "utils.h"
#include <cassert>
#include <exception>

//-------------------------------------------------------------------------------------------------
NadaFunctionTable::NadaFunctionTable() {}

//-------------------------------------------------------------------------------------------------
bool NadaFunctionTable::bind(const std::string &name, const NadaFncParameters &parameters, NadaFncCallback cb)
{
    std::string lowerName = Nada::toLower(name);

    NadaOverloadedFunction &variants = mFunctions[lowerName];
    variants.functionName = lowerName;

    // TODO: check if already there..
    variants.overloads.push_back(NadaFunctionEntry{"",parameters,std::move(cb)});

    return true;
}

//-------------------------------------------------------------------------------------------------
bool NadaFunctionTable::contains(const std::string &name, const NadaValues &parameters)
{
    std::string lowerName = Nada::toLower(name);
    if (mFunctions.count(lowerName) <= 0)
        return false;

    NadaOverloadedFunction &variants = mFunctions[lowerName];
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
NadaFunctionEntry &NadaFunctionTable::symbol(const std::string &name, const NadaValues &parameters)
{
    std::string lowerName = Nada::toLower(name);

    NadaOverloadedFunction &variants = mFunctions[lowerName];
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

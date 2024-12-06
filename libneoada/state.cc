#include "utils.h"
#include "state.h"
#include "value.h"

//-------------------------------------------------------------------------------------------------
NadaState::NadaState()
{}

//-------------------------------------------------------------------------------------------------
NadaState::~NadaState()
{
}

//-------------------------------------------------------------------------------------------------
bool NadaState::declareGlobal(const std::string &name, const std::string &typeName)
{
    Nada::Type t = Nada::typeByString(typeName);
    if (t == Nada::Undefined)
        return false;

    return mGlobals.add(NadaSymbol(t,Nada::toLower(name)));
}

//-------------------------------------------------------------------------------------------------
Nada::Type NadaState::typeOfGlobal(const std::string &name) const
{
    NadaSymbol symbol;
    if (!mGlobals.get(Nada::toLower(name),symbol))
        return Nada::Undefined;
    return symbol.type;
}

//-------------------------------------------------------------------------------------------------
bool NadaState::bind(const std::string &name, const NadaFncParameters &parameters, NadaFncCallback cb)
{
    return mFunctions.bind(name,parameters,cb);
}

//-------------------------------------------------------------------------------------------------
bool NadaState::hasFunction(const std::string &name, const NadaValues &parameters)
{
    return mFunctions.contains(name,parameters);
}

//-------------------------------------------------------------------------------------------------
NadaFunctionEntry &NadaState::function(const std::string &name, const NadaValues &parameters)
{
    return mFunctions.symbol(name,parameters);
}

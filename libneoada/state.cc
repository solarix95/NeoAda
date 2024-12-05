#include "utils.h"
#include "state.h"

//-------------------------------------------------------------------------------------------------
NadaState::NadaState()
{}

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

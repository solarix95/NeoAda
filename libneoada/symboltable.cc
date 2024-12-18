#include "symboltable.h"

//-------------------------------------------------------------------------------------------------
NadaSymbolTable::NadaSymbolTable(Scope s)
    : mScope(s)
{}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::contains(const std::string& name) const {
    return mTable.find(name) != mTable.end();
}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::add(const NadaSymbol &symbol)
{
    if (contains(symbol.name.lowerValue))
        return false;
    mTable[symbol.name.lowerValue] = symbol;
    mTable[symbol.name.lowerValue].value = new NadaValue();
    mTable[symbol.name.lowerValue].value->initType(symbol.type);
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::get(const std::string &name, NadaSymbol &symbol) const
{
    if (mTable.empty())
        return false;

    try {
        symbol = mTable.at(name);
        return true;
    } catch(...) {
        return false;
    }

    return true;
}


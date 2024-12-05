#include "symboltable.h"

NadaSymbolTable::NadaSymbolTable() {}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::contains(const std::string& name) const {
    return mTable.find(name) != mTable.end();
}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::add(const NadaSymbol &symbol)
{
    if (contains(symbol.name))
        return false;
    mTable[symbol.name] = symbol;
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::get(const std::string &name, NadaSymbol &symbol) const
{
    if (!contains(name))
        return false;
    symbol = mTable.at(name);
    return true;
}

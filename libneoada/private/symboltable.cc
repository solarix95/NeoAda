#include "symboltable.h"

//-------------------------------------------------------------------------------------------------
NadaSymbolTable::NadaSymbolTable(Scope s)
    : mScope(s)
{}

//-------------------------------------------------------------------------------------------------
NadaSymbolTable::~NadaSymbolTable()
{
    for (auto& pair : mTable) {
        delete pair.second.value;
    }
}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::contains(const std::string& name) const {
    return mTable.find(name) != mTable.end();
}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::add(const Nda::Symbol &symbol)
{
    if (contains(symbol.name.lowerValue))
        return false;
    mTable[symbol.name.lowerValue] = symbol;
    mTable[symbol.name.lowerValue].value = new NdaVariant();
    mTable[symbol.name.lowerValue].value->initType(symbol.type);
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::get(const std::string &name, Nda::Symbol &symbol) const
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


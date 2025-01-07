#include "symboltable.h"

//-------------------------------------------------------------------------------------------------
NadaSymbolTable::NadaSymbolTable(Scope s)
    : mScope(s)
{}

//-------------------------------------------------------------------------------------------------
NadaSymbolTable::~NadaSymbolTable()
{
    /*
    for (auto& pair : mTable) {
        delete pair.second->value;
        delete pair.second;
    }
    */
    for (auto symbol : mTable) {
        delete symbol->value;
        delete symbol;
    }
}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::contains(const std::string& name) const {

    // return mTable.find(name) != mTable.end();
    for (int i=0; i<mTable.size(); i++)
        if (mTable.at(i)->name.lowerValue == name)
            return true;
    return false;
}

//-------------------------------------------------------------------------------------------------
int NadaSymbolTable::indexOf(const std::string &name) const
{
    for (int i=0; i<mTable.size(); i++)
        if (mTable.at(i)->name.lowerValue == name)
            return i;
    return -1;
}

//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::add(const Nda::Symbol &symbol)
{
    if (contains(symbol.name.lowerValue))
        return false;

    mTable.push_back(new Nda::Symbol());

    mTable.back()->name = symbol.name;
    mTable.back()->type = symbol.type;
    mTable.back()->value = new NdaVariant();
    mTable.back()->value->initType(symbol.type);

    /*
    mTable[symbol.name.lowerValue] = new Nda::Symbol();
    mTable[symbol.name.lowerValue]->name = symbol.name;
    mTable[symbol.name.lowerValue]->type = symbol.type;
    mTable[symbol.name.lowerValue]->value = new NdaVariant();
    mTable[symbol.name.lowerValue]->value->initType(symbol.type);
*/
    return true;
}

//-------------------------------------------------------------------------------------------------
/*
bool NadaSymbolTable::get(const std::string &name, Nda::Symbol &symbol) const
{
    if (mTable.empty())
        return false;

    try {
        symbol = *mTable.at(name);
        return true;
    } catch(...) {
        return false;
    }

    return true;
}
*/


//-------------------------------------------------------------------------------------------------
bool NadaSymbolTable::get2(const std::string &name, Nda::Symbol **symbol) const
{
    /*
    if (mTable.empty())
        return false;

    try {
        *symbol = mTable.at(name);
        return true;
    } catch(...) {
        return false;
    }
    */

    for (int i=0; i<mTable.size(); i++)
        if (mTable.at(i)->name.lowerValue == name) {
            *symbol = mTable.at(i);
            return true;
        }

    return false;
}

//-------------------------------------------------------------------------------------------------
void NadaSymbolTable::lookUp(int index, Nda::Symbol **symbol) const
{
    *symbol = mTable.at(index);
}

#ifndef STATE_H
#define STATE_H

#include <string>
#include "symboltable.h"

class NadaState
{
public:
    NadaState();

    bool       declareGlobal(const std::string &name, const std::string &typeName);
    Nada::Type typeOfGlobal(const std::string &name) const;

private:
    NadaSymbolTable mGlobals;
};

#endif // STATE_H

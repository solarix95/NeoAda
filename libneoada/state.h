#ifndef STATE_H
#define STATE_H

#include <string>
#include "symboltable.h"
#include "functiontable.h"

class NadaState
{
public:
    NadaState();
    virtual ~NadaState();

    bool       declareGlobal(const std::string &name, const std::string &typeName);
    Nada::Type typeOfGlobal(const std::string &name) const;

    bool               bind(const std::string &name, const NadaFncParameters &parameters, NadaFncCallback cb);
    bool               hasFunction(const std::string &name, const NadaValues &parameters);
    NadaFunctionEntry &function(const std::string &name, const NadaValues &parameters);

private:
    NadaSymbolTable   mGlobals;
    NadaFunctionTable mFunctions;
};

#endif // STATE_H

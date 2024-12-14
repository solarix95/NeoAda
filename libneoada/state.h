#ifndef STATE_H
#define STATE_H

#include <memory>
#include <string>
#include "symboltable.h"
#include "functiontable.h"

class NadaState
{
public:
    NadaState();
    virtual ~NadaState();

    void       reset();

    bool       define(const std::string &name, const std::string &typeName);
    Nada::Type typeOf(const std::string &name) const;

    bool               bind(const std::string &name, const NadaFncParameters &parameters, NadaFncCallback cb);
    bool               hasFunction(const std::string &name, const NadaValues &parameters);
    NadaFunctionEntry &function(const std::string &name, const NadaValues &parameters);

    bool               find(const std::string &symbolName,NadaSymbol &symbol) const;

    NadaValue          value(const std::string &symbolName) const;
    NadaValue         &valueRef(const std::string &symbolName);

    // local scope.. as if/while/for/..
    void               pushScope();
    void               popScope();

    // callstack.. enter and leave function/procedure/method
    void               pushStack();
    void               popStack();

private:
    NadaSymbolTables   mGlobals;
    NadaFunctionTable  mFunctions;


    NadaStackFrames    mCallStack;
};

#endif // STATE_H

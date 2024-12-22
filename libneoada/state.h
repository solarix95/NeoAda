#ifndef STATE_H
#define STATE_H

#include <string>
#include "symboltable.h"
#include "functiontable.h"
#include "parser.h"

class NadaState
{
public:
    NadaState();
    virtual ~NadaState();

    void       reset();

    bool       define(const std::string &name, const std::string &typeName);
    Nada::Type typeOf(const std::string &name) const;

    bool               bind(const std::string &name, const NadaFncParameters &parameters, NadaFncCallback cb);
    bool               bind(const std::string &name, const NadaFncParameters &parameters, const std::shared_ptr<NadaParser::ASTNode> &block);
    bool               hasFunction(const std::string &name, const NadaValues &parameters);
    NadaFunctionEntry &function(const std::string &name, const NadaValues &parameters);

    bool               find(const std::string &symbolName,NadaSymbol &symbol) const;

    NadaValue          value(const std::string &symbolName) const;
    NadaValue         &valueRef(const std::string &symbolName);
    NadaValue         *valuePtr(const std::string &symbolName);


    // local scope.. as if/while/for/..
    void               pushScope(NadaSymbolTable::Scope s);
    void               popScope();

    // callstack.. enter and leave function/procedure/method
    void               pushStack(NadaSymbolTable::Scope s);
    void               popStack();

    bool               inLoopScope() const;
    bool               inLoopScope(const NadaSymbolTables &tables) const;


    inline NadaValue  &ret() { return mRetValue; }

private:
    NadaValue          mRetValue;
    NadaSymbolTables   mGlobals;
    NadaFunctionTable  mFunctions;


    NadaStackFrames    mCallStack;
};

#endif // STATE_H

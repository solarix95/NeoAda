#ifndef STATE_H
#define STATE_H

#include <string>
#include "private/symboltable.h"
#include "private/functiontable.h"
#include "parser.h"


class NadaState
{
public:
    NadaState();
    virtual ~NadaState();

    void       reset();

    bool       define(const std::string &name, const std::string &typeName, bool isVolatile = false);
    Nada::Type typeOf(const std::string &name) const;

    bool               bind(const std::string &name, const NadaFncParameters &parameters, NadaFncCallback cb);
    bool               bind(const std::string &name, const NadaFncParameters &parameters, const std::shared_ptr<NadaParser::ASTNode> &block);
    bool               hasFunction(const std::string &name, const NadaValues &parameters);
    NadaFunctionEntry &function(const std::string &name, const NadaValues &parameters);

    bool               find(const std::string &symbolName,NadaSymbol &symbol) const;

    NadaValue          value(const std::string &symbolName) const;
    NadaValue         &valueRef(const std::string &symbolName);
    NadaValue         *valuePtr(const std::string &symbolName);

    // Volatile interface
    // Volatile Callbacks
    using CtorCallback   = std::function<void     (const std::string &symbolName, NadaValue &value)>;
    using DtorCallback   = std::function<void     (NadaValue &value)>;
    using ReadCallback   = std::function<bool     (NadaValue &value)>;
    using WriteCallback  = std::function<bool     (NadaValue &value)>;

    void  onVolatileCtor (CtorCallback  cb);

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

    CtorCallback       mVolatileCtor;
};

#endif // STATE_H

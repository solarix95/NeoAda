#ifndef NEOADA_STATE_H
#define NEOADA_STATE_H

#include <string>
#include "private/symboltable.h"
#include "private/functiontable.h"
#include "parser.h"

class NdaState
{
public:
    NdaState();
    virtual ~NdaState();

    void       reset();

    // runtime type information
    bool                    registerType(std::string name, Nda::Type type, bool instantiable);
    bool                    registerType(std::string name, std::string basename); // "type mytype as list;"
    const Nda::RuntimeType *typeByName(std::string name) const;

    // variable definition
    bool       define(const std::string &name, const std::string &typeName, bool isVolatile = false);
    bool       define(const std::string &name, const Nda::RuntimeType *type, bool isVolatile = false);
    Nda::Type  typeOf(const std::string &name) const;

    // procedure/function
    bool               bindFnc(const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb); // function
    bool               bindPrc(const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb); // procedure
    bool               bind(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, const std::shared_ptr<NadaParser::ASTNode> &block);
    bool               hasFunction(const std::string &type, const std::string &name, const NadaValues &parameters);
    Nda::FunctionEntry &function(const std::string &type, const std::string &name, const NadaValues &parameters);

    // methods
    bool               bindFnc(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb);
    bool               bindPrc(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb);

    bool               find(const std::string &symbolName,Nda::Symbol &symbol) const;

    NdaVariant          value(const std::string &symbolName) const;
    NdaVariant         &valueRef(const std::string &symbolName);
    NdaVariant         *valuePtr(const std::string &symbolName);

    // Volatile interface
    // Volatile Callbacks
    using CtorCallback   = std::function<void     (const std::string &symbolName, NdaVariant &value)>;
    using DtorCallback   = std::function<void     (NdaVariant &value)>;
    using ReadCallback   = std::function<bool     (NdaVariant &value)>;
    using WriteCallback  = std::function<bool     (NdaVariant &value)>;

    void  onVolatileCtor (CtorCallback  cb);

    // "With" Addons
    using WithCallback  = std::function<void(std::string &addonName)>;
    void  onWith(WithCallback cb);
    void  requestAddon(std::string name);

    // local scope.. as if/while/for/..
    void               pushScope(NadaSymbolTable::Scope s);
    void               popScope();

    // callstack.. enter and leave function/procedure/method
    void               pushStack(NadaSymbolTable::Scope s);
    void               popStack();

    bool               inLoopScope() const;
    bool               inLoopScope(const NadaSymbolTables &tables) const;


    inline NdaVariant  &ret() { return mRetValue; }

private:
    void destroy();

    NdaVariant         mRetValue;
    NadaSymbolTables   mGlobals;
    NadaStackFrames    mCallStack;

    Nda::FunctionTable  mFunctions;
    Nda::RuntimeTypes   mTypes;

    CtorCallback       mVolatileCtor;

    WithCallback       mWithCallback;
};

#endif // STATE_H

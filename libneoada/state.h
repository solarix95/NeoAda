#ifndef LIB_NEOADA_STATE_H
#define LIB_NEOADA_STATE_H

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
    const Nda::RuntimeType *registerType(std::string name, Nda::Type type, bool instantiable);
    const Nda::RuntimeType *registerType(std::string name, std::string basename); // "type mytype as list;"
    const Nda::RuntimeType *typeByName(std::string name) const;

    // cache.. just for performance reasons:
    inline const Nda::RuntimeType *booleanType() const   { return mBooleanType; }
    inline const Nda::RuntimeType *numberType() const    { return mNumberType; }
    inline const Nda::RuntimeType *naturalType() const   { return mNaturalType; }
    inline const Nda::RuntimeType *stringType() const    { return mStringType; }
    inline const Nda::RuntimeType *listType() const      { return mListType; }
    inline const Nda::RuntimeType *referenceType() const { return mReferenceType; }

    // variable definition
    bool       define(const std::string &name, const std::string &typeName, bool isVolatile = false);
    bool       define(const std::string &name, const Nda::RuntimeType *type, bool isVolatile = false);
    Nda::Type  typeOf(const std::string &name) const;

    // procedure/function
    bool               bindFnc(const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb); // function
    bool               bindPrc(const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb); // procedure
    bool               bind(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, const std::shared_ptr<NdaParser::ASTNode> &block);
    bool               bind(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::Runnable *block);
    bool               hasFunction(const std::string &type, const std::string &name, const NadaValues &parameters);
    Nda::FunctionEntry &function(const std::string &type, const std::string &name, const NadaValues &parameters);

    // methods
    bool               bindFnc(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb);
    bool               bindPrc(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb);

    bool               find(const std::string &symbolName,Nda::Symbol **symbol) const;
    bool               find(const std::string &symbolName,int &index, int &scope, bool &isGlobal) const;

    NdaVariant          value(const std::string &symbolName) const;
    NdaVariant         &valueRef(const std::string &symbolName);
    NdaVariant         *valuePtr(const std::string &symbolName);
    NdaVariant         *valuePtr(int index, int scope, bool isGlobal);

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

    inline NdaVariant  &ret()  { return mRetValue; }

private:
    void destroy();

    NdaVariant         mRetValue;


    NadaSymbolTables   mGlobals;
    NadaStackFrames    mCallStack;

    Nda::FunctionTable  mFunctions;
    Nda::RuntimeTypes   mTypes;

    CtorCallback       mVolatileCtor;

    WithCallback       mWithCallback;

    // cache
    const Nda::RuntimeType *mBooleanType;
    const Nda::RuntimeType *mNumberType;
    const Nda::RuntimeType *mNaturalType;
    const Nda::RuntimeType *mStringType;
    const Nda::RuntimeType *mListType;
    const Nda::RuntimeType *mReferenceType;
};

#endif // STATE_H

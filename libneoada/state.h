#ifndef LIB_NEOADA_STATE_H
#define LIB_NEOADA_STATE_H

#include <string>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "private/symboltable.h"
#include "private/functiontable.h"
#include "parser.h"
#include "value.h"

class NdaInterpreter;

namespace Nda {

struct SourceLocation {
    uint32_t sourceId;
    int line;
    int column;

    SourceLocation(uint32_t source = 0, int row = 0, int col = 0)
        : sourceId(source), line(row), column(col) {}

    bool isValid() const { return sourceId != 0 || line != 0 || column != 0; }
};

struct StackFrame {
    std::string callable;
    SourceLocation callSite;

    StackFrame(const std::string &name = "", const SourceLocation &site = SourceLocation())
        : callable(name), callSite(site) {}
};

struct ExceptionContext {
    std::string name;
    std::string message;
    SourceLocation origin;
    std::vector<StackFrame> stack;

    bool isValid() const { return !name.empty(); }
    void clear() { name.clear(); message.clear(); origin = SourceLocation(); stack.clear(); }
};

}

class NdaState
{
public:
    NdaState();
    virtual ~NdaState();

    void       reset();

    uint32_t registerSource(const std::string &sourceName, const std::string &source = "", const std::string &kind = "script");
    std::string sourceName(uint32_t sourceId) const;
    static std::string anonymousSourceName(const std::string &source, const std::string &kind = "script", size_t limit = 100);

    const Nda::SourceLocation &currentLocation() const;
    void setCurrentLocation(const Nda::SourceLocation &location);

    void pushDebugFrame(const std::string &callable, const Nda::SourceLocation &callSite);
    void popDebugFrame();

    void raiseException(const std::string &name, const std::string &message = "");
    const Nda::ExceptionContext &pendingException() const;
    void setPendingException(const Nda::ExceptionContext &context);
    void clearPendingException();
    const Nda::ExceptionContext *activeException() const;
    void pushActiveException(const Nda::ExceptionContext &context);
    void popActiveException();

    std::string exceptionWhat() const;
    std::string exceptionWhere() const;
    std::string exceptionTrace() const;

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
    inline const Nda::RuntimeType *bytesType() const     { return mBytesType; }
    inline const Nda::RuntimeType *dictType() const      { return mDictType; }
    inline const Nda::RuntimeType *referenceType() const { return mReferenceType; }

    // variable definition
    bool       define(const std::string &name, const std::string &typeName, bool isVolatile = false);
    bool       define(const std::string &name, const Nda::RuntimeType *type, bool isVolatile = false);
    Nda::Type  typeOf(const std::string &name) const;

    // procedure/function
    bool               bindFnc(const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb); // function
    bool               bindPrc(const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb); // procedure
    bool               bind(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, const std::shared_ptr<NdaParser::ASTNode> &block, const std::string &returnType = "");
    bool               bind(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::Runnable *block, const std::string &returnType = "");
    bool               hasFunction(const std::string &type, const std::string &name, const NdaVariants &parameters);
    Nda::FunctionEntry *functionPtr(const std::string &type, const std::string &name, const NdaVariants &parameters);
    Nda::FunctionEntry &function(const std::string &type, const std::string &name, const NdaVariants &parameters);

    // methods
    bool               bindFnc(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb);
    bool               bindPrc(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb);

    bool               find(const std::string &symbolName,Nda::Symbol **symbol) const;
    bool               find(const std::string &symbolName,int &index, int &scope, bool &isGlobal) const;

    // Variant lookup
    NdaVariant          value(const std::string &symbolName) const;
    NdaVariant         &valueRef(const std::string &symbolName);
    NdaVariant         *valuePtr(const std::string &symbolName);
    Nda::Symbol        *symbolPtr(int index, int scope, bool isGlobal);
    NdaVariant         *valuePtr(int index, int scope, bool isGlobal);

    // variant to value and vice versa
    NdaVariant          toVariant(const NdaValue &value) const;
    NdaValue            toValue(const NdaVariant &value) const;

    // Volatile interface
    // Volatile Callbacks
    using CtorCallback   = std::function<void     (const std::string &symbolName, NdaVariant &value)>;
    using DtorCallback   = std::function<void     (NdaVariant &value)>;
    using ReadCallback   = std::function<bool     (NdaVariant &value)>;
    using WriteCallback  = std::function<bool     (const NdaVariant &newValue)>;
    using WriteIndexCallback  = std::function<bool     (const NdaVariant &index, const NdaVariant &newValue)>;
    using ReadIndexCallback   = std::function<bool (const NdaVariant &index,NdaVariant &value)>;

    void  onVolatileCtor (CtorCallback  cb);
    void  onVolatileRead (const std::string &symbolname, ReadCallback  cb);      // natural/number/bool/...
    void  onVolatileRead (const std::string &symbolname, ReadIndexCallback  cb); // list/dict
    void  onVolatileWrite(const std::string &symbolname, WriteCallback  cb);
    void  onVolatileWrite(const std::string &symbolname, WriteIndexCallback  cb);
    bool  readVolatile   (const std::string &symbolname, NdaVariant &value);
    bool  writeVolatile  (const std::string &symbolname, const NdaVariant &value);
    bool  readVolatile   (const std::string &symbolname, const NdaVariant &index, NdaVariant &value);
    bool  writeVolatile  (const std::string &symbolname, const NdaVariant &index, const NdaVariant &value);

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

    std::vector<std::string> globalFunctions() const;

    inline NdaVariant  &ret()  { return mRetValue; }

    inline std::string  unhandledException() const { return mPendingException.name; }
    inline bool         hasUnhandledException() const { return mPendingException.isValid(); }

private:
    friend class NdaInterpreter;

    inline void         setUnhandledException(const std::string &name) { raiseException(name); }
    inline void         clearUnhandledException() { clearPendingException(); }

    void destroy();

    NdaVariant         mRetValue;
    Nda::ExceptionContext mPendingException;
    std::vector<Nda::ExceptionContext> mActiveExceptions;
    Nda::SourceLocation mCurrentLocation;
    std::vector<Nda::StackFrame> mDebugCallStack;
    uint32_t mNextSourceId;
    std::unordered_map<uint32_t, std::string> mSources;

    NadaSymbolTables   mGlobals;
    NadaStackFrames    mCallStack;

    Nda::FunctionTable  mFunctions;
    Nda::RuntimeTypes   mTypes;

    CtorCallback       mVolatileCtor;
    std::unordered_map<std::string, ReadCallback> mVolatileReads;
    std::unordered_map<std::string, ReadIndexCallback> mVolatileIndexReads;
    std::unordered_map<std::string, WriteCallback> mVolatileWrites;
    std::unordered_map<std::string, WriteIndexCallback> mVolatileIndexWrites;

    WithCallback       mWithCallback;
    std::unordered_set<std::string> mLoadedAddons;

    // cache
    const Nda::RuntimeType *mBooleanType;
    const Nda::RuntimeType *mNumberType;
    const Nda::RuntimeType *mNaturalType;
    const Nda::RuntimeType *mStringType;
    const Nda::RuntimeType *mListType;
    const Nda::RuntimeType *mBytesType;
    const Nda::RuntimeType *mDictType;
    const Nda::RuntimeType *mReferenceType;
};

#endif // STATE_H

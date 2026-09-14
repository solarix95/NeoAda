#include "private/utils.h"
#include "state.h"
#include "variant.h"
#include <cassert>
#include <cctype>
#include <sstream>

#define BUILD_METHOD(t,n) (t + ":" + n)

//-------------------------------------------------------------------------------------------------
NdaState::NdaState()
    : mNextSourceId(1)
    , mBooleanType(nullptr)
    , mNumberType(nullptr)
    , mNaturalType(nullptr)
    , mStringType(nullptr)
    , mListType(nullptr)
    , mBytesType(nullptr)
    , mReferenceType(nullptr)
{
    reset();
}

//-------------------------------------------------------------------------------------------------
NdaState::~NdaState()
{
    destroy();
}

//-------------------------------------------------------------------------------------------------
void NdaState::reset()
{
    destroy();

    mPendingException.clear();
    mActiveExceptions.clear();
    mCurrentLocation = Nda::SourceLocation();
    mDebugCallStack.clear();

    mGlobals.push_back(new NadaSymbolTable(NadaSymbolTable::GlobalScope));

    // register all standard NeoAda Datatypes
    mReferenceType = registerType("Reference",Nda::Reference, false); assert(mReferenceType);
    registerType("Any",Nda::Any, true);
    mNumberType = registerType("Number",Nda::Number, true); assert(mNumberType);

    mNaturalType = registerType("Natural",Nda::Natural, true); assert(mNaturalType);
    registerType("Supernatural",Nda::Supernatural, true);
    registerType("Byte",Nda::Byte, true);

    mBooleanType = registerType("Boolean",Nda::Boolean, true); assert(mBooleanType);
    mStringType  = registerType("String",Nda::String, true);   assert(mStringType);
    mListType    = registerType("List",Nda::List, true);       assert(mListType);
    mBytesType   = registerType("Bytes",Nda::Bytes, true);     assert(mBytesType);
    mDictType    = registerType("Dict",Nda::Dict, true);       assert(mDictType);

    bindFnc("typeof", {{"value", "Any", Nda::InMode}}, [this](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        const auto type = args.at("value").runtimeType();
        if (!type)
            return false;

        ret.fromString(stringType(), type->name.lowerValue);
        return true;
    });

    bindFnc("isany", {{"value", "Any", Nda::InMode}}, [this](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        const auto type = args.at("value").runtimeType();
        ret.fromBool(booleanType(), type && type->dataType == Nda::Any);
        return true;
    });

    bindFnc("what", {}, [this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromString(stringType(), exceptionWhat());
        return true;
    });

    bindFnc("where", {}, [this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromString(stringType(), exceptionWhere());
        return true;
    });

    bindFnc("trace", {}, [this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromString(stringType(), exceptionTrace());
        return true;
    });

}

//-------------------------------------------------------------------------------------------------
std::string NdaState::anonymousSourceName(const std::string &source, const std::string &kind, size_t limit)
{
    std::string preview;
    bool previousWasSpace = false;
    bool truncated = false;

    for (unsigned char c : source) {
        if (std::isspace(c)) {
            if (!preview.empty() && !previousWasSpace)
                preview += " ";
            previousWasSpace = true;
            continue;
        }
        previousWasSpace = false;
        if (std::iscntrl(c))
            continue;
        std::string encoded;
        if (c == 92 || c == 34)
            encoded += static_cast<char>(92);
        encoded += static_cast<char>(c);
        if (preview.size() + encoded.size() > limit) {
            truncated = true;
            break;
        }
        preview += encoded;
    }

    while (!preview.empty() && preview.back() == 32)
        preview.pop_back();
    if (preview.empty())
        return "<" + kind + ">";
    return "[" + kind + " \"" + preview + (truncated ? "..." : "") + "\"]";
}

//-------------------------------------------------------------------------------------------------
uint32_t NdaState::registerSource(const std::string &requestedName, const std::string &source, const std::string &kind)
{
    const std::string displayName = requestedName.empty() ? anonymousSourceName(source, kind) : requestedName;
    for (const auto &entry : mSources) {
        if (entry.second == displayName)
            return entry.first;
    }
    const uint32_t id = mNextSourceId++;
    mSources[id] = displayName;
    return id;
}

//-------------------------------------------------------------------------------------------------
std::string NdaState::sourceName(uint32_t sourceId) const
{
    const auto it = mSources.find(sourceId);
    return it == mSources.end() ? std::string("<unknown>") : it->second;
}

//-------------------------------------------------------------------------------------------------
const Nda::SourceLocation &NdaState::currentLocation() const
{
    return mCurrentLocation;
}

//-------------------------------------------------------------------------------------------------
void NdaState::setCurrentLocation(const Nda::SourceLocation &location)
{
    mCurrentLocation = location;
}

//-------------------------------------------------------------------------------------------------
void NdaState::pushDebugFrame(const std::string &callable, const Nda::SourceLocation &callSite)
{
    mDebugCallStack.push_back({callable, callSite});
}

//-------------------------------------------------------------------------------------------------
void NdaState::popDebugFrame()
{
    assert(!mDebugCallStack.empty());
    mDebugCallStack.pop_back();
}

//-------------------------------------------------------------------------------------------------
void NdaState::raiseException(const std::string &name, const std::string &message)
{
    mPendingException.name = Nda::toLower(name);
    mPendingException.message = message;
    mPendingException.origin = mCurrentLocation;
    mPendingException.stack = mDebugCallStack;
}

//-------------------------------------------------------------------------------------------------
const Nda::ExceptionContext &NdaState::pendingException() const
{
    return mPendingException;
}

//-------------------------------------------------------------------------------------------------
void NdaState::setPendingException(const Nda::ExceptionContext &context)
{
    mPendingException = context;
}

//-------------------------------------------------------------------------------------------------
void NdaState::clearPendingException()
{
    mPendingException.clear();
}

//-------------------------------------------------------------------------------------------------
const Nda::ExceptionContext *NdaState::activeException() const
{
    return mActiveExceptions.empty() ? nullptr : &mActiveExceptions.back();
}

//-------------------------------------------------------------------------------------------------
void NdaState::pushActiveException(const Nda::ExceptionContext &context)
{
    mActiveExceptions.push_back(context);
}

//-------------------------------------------------------------------------------------------------
void NdaState::popActiveException()
{
    assert(!mActiveExceptions.empty());
    mActiveExceptions.pop_back();
}

//-------------------------------------------------------------------------------------------------
std::string NdaState::exceptionWhere() const
{
    const Nda::ExceptionContext *context = activeException();
    if (!context && mPendingException.isValid())
        context = &mPendingException;
    if (!context)
        return "";
    const auto &loc = context->origin;
    return sourceName(loc.sourceId) + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.column);
}

//-------------------------------------------------------------------------------------------------
std::string NdaState::exceptionWhat() const
{
    const Nda::ExceptionContext *context = activeException();
    if (!context && mPendingException.isValid())
        context = &mPendingException;
    if (!context)
        return "";
    return context->message.empty() ? context->name : context->name + ": " + context->message;
}

//-------------------------------------------------------------------------------------------------
std::string NdaState::exceptionTrace() const
{
    const Nda::ExceptionContext *context = activeException();
    if (!context && mPendingException.isValid())
        context = &mPendingException;
    if (!context)
        return "";

    std::ostringstream out;
    out << exceptionWhat() << "\n  at " << sourceName(context->origin.sourceId) << ":" << context->origin.line << ":" << context->origin.column;
    for (auto it = context->stack.rbegin(); it != context->stack.rend(); ++it)
        out << "\n  called from " << it->callable << " at " << sourceName(it->callSite.sourceId) << ":" << it->callSite.line << ":" << it->callSite.column;
    return out.str();
}

//-------------------------------------------------------------------------------------------------
const Nda::RuntimeType *NdaState::registerType(std::string name, Nda::Type type, bool instantiable)
{
    Nda::LowerString lname(name);
    const auto *currentType = typeByName(lname.lowerValue);
    if (currentType) {
        if (currentType->dataType == type) // already registered -> ok..
            return currentType;
    }

    mTypes[lname.lowerValue] = Nda::RuntimeType(lname,type,"",instantiable);
    return &mTypes[lname.lowerValue];
}

//-------------------------------------------------------------------------------------------------
const Nda::RuntimeType *NdaState::registerType(std::string name, std::string basename)
{
    name = Nda::toLower(name);
    if (mTypes.find(name) != mTypes.end())
        return nullptr;

    const auto *baseType = typeByName(basename);
    if (!baseType)
        return nullptr;

    if (baseType->instantiable == false) // dont subclass "Reference"!!
        return nullptr;

    mTypes[name] = Nda::RuntimeType(name,baseType->dataType,basename,true);
    return &mTypes.at(name);
}

//-------------------------------------------------------------------------------------------------
const Nda::RuntimeType *NdaState::typeByName(std::string name) const
{
    if (mTypes.find(name) == mTypes.end())
        return nullptr;

    return &mTypes.at(name);
}


//-------------------------------------------------------------------------------------------------
bool NdaState::define(const std::string &name, const std::string &typeName, bool isVolatile)
{
    const Nda::RuntimeType *t = typeByName(Nda::toLower(typeName));
    if (!t || !t->instantiable)
        return false;

    return define(name,t,isVolatile);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::define(const std::string &name, const Nda::RuntimeType *type, bool isVolatile)
{
    assert(type);

    bool done;
    if (mCallStack.empty())
        done = mGlobals.back()->add(Nda::Symbol(name, type, isVolatile));
    else
        done = mCallStack.back()->back()->add(Nda::Symbol(name, type, isVolatile));

    if (done && isVolatile) {
        NdaVariant &value = valueRef(name);
        if (mVolatileCtor)
            mVolatileCtor(name,value);
    }

    return done;

}

//-------------------------------------------------------------------------------------------------
Nda::Type NdaState::typeOf(const std::string &name) const
{
    Nda::Symbol *symbol;
    if (!find(name,&symbol))
        return Nda::Undefined;
    return symbol->type->dataType;
}

//-------------------------------------------------------------------------------------------------
bool NdaState::bindFnc(const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb)
{
    assert(!name.empty());
    return mFunctions.bindFnc(name,parameters,cb);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::bindPrc(const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb)
{
    assert(!name.empty());
    return mFunctions.bindPrc(name,parameters,std::move(cb));
}

//-------------------------------------------------------------------------------------------------
bool NdaState::bind(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, const std::shared_ptr<NdaParser::ASTNode> &block, const std::string &returnType)
{
    assert(!name.empty());
    return mFunctions.bind(type.empty() ? name : BUILD_METHOD(type,name),parameters,block,returnType);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::bind(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::Runnable *block, const std::string &returnType)
{
    assert(!name.empty());
    return mFunctions.bind(type.empty() ? name : BUILD_METHOD(type,name),parameters,block,returnType);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::hasFunction(const std::string &type, const std::string &name, const NdaVariants &parameters)
{
    return functionPtr(type, name, parameters) != nullptr;
}

//-------------------------------------------------------------------------------------------------
Nda::FunctionEntry *NdaState::functionPtr(const std::string &type, const std::string &name, const NdaVariants &parameters)
{
    return mFunctions.symbolPtr(type.empty() ? name : BUILD_METHOD(type,name),parameters);
}

//-------------------------------------------------------------------------------------------------
Nda::FunctionEntry &NdaState::function(const std::string &type, const std::string &name, const NdaVariants &parameters)
{
    return mFunctions.symbol(type.empty() ? name : BUILD_METHOD(type,name),parameters);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::bindFnc(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb)
{
    assert(!type.empty());
    assert(!name.empty());
    return bindFnc(BUILD_METHOD(type,name), parameters, std::move(cb));
}

//-------------------------------------------------------------------------------------------------
bool NdaState::bindPrc(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb)
{
    assert(!type.empty());
    assert(!name.empty());
    return bindPrc(BUILD_METHOD(type,name), parameters, std::move(cb));
}

//-------------------------------------------------------------------------------------------------
bool NdaState::find(const std::string &symbolName, Nda::Symbol **symbol) const
{
    // First priority: current function scope:
    if (!mCallStack.empty()) {
        const auto& currentFrame = mCallStack.back();

        /*
        for (auto it = currentFrame->rbegin(); it != currentFrame->rend(); ++it) {
            if ((*it)->get2(symbolName,symbol)) {
                return true;
            }
        }
        */
        for (int i=(*currentFrame).size()-1; i >= 0; i-- ) {
            if ((*currentFrame)[i]->get2(symbolName,symbol))
                return true;
        }


    }

    // second: globals:
    /*
    for (auto it = mGlobals.rbegin(); it != mGlobals.rend(); ++it) {
        if ((*it)->get2(symbolName,symbol)) {
            return true;
        }
    }
    */
    for (int i=mGlobals.size()-1; i >= 0; i-- ) {
        if (mGlobals[i]->get2(symbolName,symbol))
            return true;
    }


    return false;
}

//-------------------------------------------------------------------------------------------------
bool NdaState::find(const std::string &symbolName, int &index, int &scope, bool &isGlobal) const
{
    // First priority: current function scope:
    if (!mCallStack.empty()) {
        const auto& currentFrame = mCallStack.back();

        /*
        for (auto it = currentFrame->rbegin(); it != currentFrame->rend(); ++it) {
            if ((*it)->get2(symbolName,symbol)) {
                return true;
            }
        }
        */
        for (int i=(*currentFrame).size()-1; i >= 0; i-- ) {
            if ((index = (*currentFrame)[i]->indexOf(symbolName)) >= 0) {
                scope    = i;
                isGlobal = false;
                return true;
            }
        }
    }

    // second: globals:
    /*
    for (auto it = mGlobals.rbegin(); it != mGlobals.rend(); ++it) {
        if ((*it)->get2(symbolName,symbol)) {
            return true;
        }
    }
    */
    for (int i=mGlobals.size()-1; i >= 0; i-- ) {
        if ((index = mGlobals[i]->indexOf(symbolName)) >= 0) {
            scope    = i;
            isGlobal = true;
            return true;
        }
    }

    return false;
}


//-------------------------------------------------------------------------------------------------
NdaVariant NdaState::value(const std::string &symbolName) const
{
    Nda::Symbol *symbol;
    if (!find(symbolName,&symbol))
        return NdaVariant();
    return *symbol->value;
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaState::valueRef(const std::string &symbolName)
{
    assert(!symbolName.empty());
    Nda::Symbol *symbol;
    bool done = find(symbolName,&symbol);
    assert(done);
    return *(symbol->value);
}

//-------------------------------------------------------------------------------------------------
NdaVariant *NdaState::valuePtr(const std::string &symbolName)
{
    Nda::Symbol *symbol;
    bool done = find(symbolName,&symbol);
    assert(done);
    return symbol->value;
}

//-------------------------------------------------------------------------------------------------
Nda::Symbol *NdaState::symbolPtr(int index, int scope, bool isGlobal)
{
    Nda::Symbol *symbol;

    if (isGlobal)
        mGlobals[scope]->lookUp(index,&symbol);
    else
        (*mCallStack.back())[scope]->lookUp(index,&symbol);

    return symbol;
}

//-------------------------------------------------------------------------------------------------
NdaVariant *NdaState::valuePtr(int index, int scope, bool isGlobal)
{
    return symbolPtr(index, scope, isGlobal)->value;
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaState::toVariant(const NdaValue &value) const
{
    NdaVariant ret;

    switch(value.type()) {
        case Nda::String:
            ret.fromString(stringType(),value.toString());
            break;
        case Nda::Number:
            ret.fromNumber(numberType(),value.toDouble());
            break;
        case Nda::Natural:
            ret.fromNatural(naturalType(),value.toInt64());
            break;
        case Nda::Boolean:
            ret.fromNatural(booleanType(),value.toBool());
            break;
        default:
            assert(0);
    }
    return ret;
}

//-------------------------------------------------------------------------------------------------
NdaValue NdaState::toValue(const NdaVariant &value) const
{
    switch(value.type()) {
    case Nda::String:
        return NdaValue(value.toString());
        break;
    case Nda::Number:
        return NdaValue(value.toDouble());
        break;
    case Nda::Natural:
        return NdaValue(value.toInt64());
        break;
    case Nda::Boolean:
        return NdaValue(value.toBool());
        break;
    default:
        assert(0);
    }
}

//-------------------------------------------------------------------------------------------------
void NdaState::pushStack(NadaSymbolTable::Scope s)
//                            enter Function/Procedure/Method
{
    mCallStack.push_back(new NadaSymbolTables());
    mCallStack.back()->push_back(new NadaSymbolTable(s)); // TODO: Performance.. maybe only on new local symbol?
}

//-------------------------------------------------------------------------------------------------
void NdaState::popStack()
//                            leave Function/Procedure/Method
{
    assert(mCallStack.size() > 0);
    assert(mCallStack.back()->size() == 1);

    auto *topTable = mCallStack.back()->back();
    delete topTable;
    mCallStack.back()->pop_back();

    auto *topFrame = mCallStack.back();
    delete topFrame;
    mCallStack.pop_back();
}

//-------------------------------------------------------------------------------------------------
bool NdaState::inLoopScope() const
{
    if (!mCallStack.empty())
        return inLoopScope(*mCallStack.back());
    return inLoopScope(mGlobals);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::inLoopScope(const NadaSymbolTables &tables) const
{
    for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
        if ((*it) && (*it)->scope() == NadaSymbolTable::LoopScope)
            return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
std::vector<std::string> NdaState::globalFunctions() const
{
    return mFunctions.symbolNames();
}


//-------------------------------------------------------------------------------------------------
void NdaState::destroy()
{
    mRetValue.reset(); // detach references

    mFunctions.clear(); // release shared-pointers: ASTNodes
    mLoadedAddons.clear();
    mPendingException.clear();
    mActiveExceptions.clear();
    mDebugCallStack.clear();
    mCurrentLocation = Nda::SourceLocation();
    mSources.clear();
    mNextSourceId = 1;

    while (!mCallStack.empty()) {
        auto *tables = mCallStack.back();
        while (!tables->empty()) {
            delete tables->back();
            tables->pop_back();
        }
        delete tables;
        mCallStack.pop_back();
    }

    while (!mGlobals.empty()) {
        auto *table = mGlobals.back();
        delete table;
        mGlobals.pop_back();
    }

    mTypes.clear();
}


//-------------------------------------------------------------------------------------------------
void NdaState::onVolatileCtor(NdaState::CtorCallback cb)
{
    mVolatileCtor = std::move(cb);
}

//-------------------------------------------------------------------------------------------------
void NdaState::onVolatileRead(const std::string &symbolname, ReadCallback cb)
{
    mVolatileReads[Nda::toLower(symbolname)] = std::move(cb);
}

void NdaState::onVolatileRead(const std::string &symbolname, ReadIndexCallback cb)
{
    mVolatileIndexReads[Nda::toLower(symbolname)] = std::move(cb);
}

//-------------------------------------------------------------------------------------------------
void NdaState::onVolatileWrite(const std::string &symbolname, WriteCallback cb)
{
    mVolatileWrites[Nda::toLower(symbolname)] = std::move(cb);
}

//-------------------------------------------------------------------------------------------------
void NdaState::onVolatileWrite(const std::string &symbolname, WriteIndexCallback cb)
{
    mVolatileIndexWrites[Nda::toLower(symbolname)] = std::move(cb);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::readVolatile(const std::string &symbolname, NdaVariant &value)
{
    auto it = mVolatileReads.find(Nda::toLower(symbolname));
    if (it == mVolatileReads.end())
        return false;

    return it->second(value);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::readVolatile(const std::string &symbolname, const NdaVariant &index, NdaVariant &value)
{
    auto it = mVolatileIndexReads.find(Nda::toLower(symbolname));
    if (it == mVolatileIndexReads.end())
        return false;

    return it->second(index, value);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::writeVolatile(const std::string &symbolname, const NdaVariant &value)
{
    auto it = mVolatileWrites.find(Nda::toLower(symbolname));
    if (it == mVolatileWrites.end())
        return true;

    return it->second(value);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::writeVolatile(const std::string &symbolname, const NdaVariant &index, const NdaVariant &value)
{
    auto it = mVolatileIndexWrites.find(Nda::toLower(symbolname));
    if (it == mVolatileIndexWrites.end())
        return true;

    return it->second(index, value);
}

//-------------------------------------------------------------------------------------------------
void NdaState::onWith(WithCallback cb)
{
   mWithCallback = std::move(cb);
}

//-------------------------------------------------------------------------------------------------
void NdaState::requestAddon(std::string name)
{
    if (!mWithCallback)
        return;

    name = Nda::toLower(name);
    if (mLoadedAddons.count(name) > 0)
        return;

    mLoadedAddons.insert(name);
    mWithCallback(name);
}

//-------------------------------------------------------------------------------------------------
void NdaState::pushScope(NadaSymbolTable::Scope s)
//                            enter block (if/while/...)
{
    if (mCallStack.empty())
        mGlobals.push_back(new NadaSymbolTable(s));
    else
        mCallStack.back()->push_back(new NadaSymbolTable(s));
}

//-------------------------------------------------------------------------------------------------
void NdaState::popScope()
//                            leave block (if/while/...)
{
    if (mCallStack.empty()) {
        assert(mGlobals.size() > 1); // globals are never empty..
        delete mGlobals.back();
        mGlobals.pop_back();
    } else {
        assert(mCallStack.back()->size() > 0);
        auto *frame = mCallStack.back();
        auto *scope = frame->back();
        delete scope;
        frame->pop_back();
    }
}

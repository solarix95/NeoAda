#include "private/utils.h"
#include "state.h"
#include "variant.h"
#include <cassert>

#define BUILD_METHOD(t,n) (t + ":" + n)

//-------------------------------------------------------------------------------------------------
NdaState::NdaState()
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

    mGlobals.push_back(new NadaSymbolTable(NadaSymbolTable::GlobalScope));

    // register all standard NeoAda Datatypes
    registerType("Reference",Nda::Reference, false);
    registerType("Any",Nda::Any, true);
    registerType("Number",Nda::Number, true);

    registerType("Natural",Nda::Natural, true);
    registerType("Supernatural",Nda::Supernatural, true);
    registerType("Byte",Nda::Byte, true);

    registerType("Boolean",Nda::Boolean, true);

    registerType("String",Nda::String, true);
    registerType("List",Nda::List, true);
    registerType("Dict",Nda::Dict, true);

}

//-------------------------------------------------------------------------------------------------
bool NdaState::registerType(std::string name, Nda::Type type, bool instantiable)
{
    Nda::LowerString lname(name);
    const auto *currentType = typeByName(lname.lowerValue);
    if (currentType) {
        return currentType->dataType == type; // already registered -> ok..
    }

    mTypes[lname.lowerValue] = Nda::RuntimeType(lname,type,"",instantiable);
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NdaState::registerType(std::string name, std::string basename)
{
    name = Nda::toLower(name);
    if (mTypes.find(name) != mTypes.end())
        return false;

    const auto *baseType = typeByName(basename);
    if (!baseType)
        return false;

    if (baseType->instantiable == false) // dont subclass "Reference"!!
        return false;

    mTypes[name] = Nda::RuntimeType(name,baseType->dataType,basename,true);
    return true;
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
        done = mGlobals.back()->add(Nda::Symbol(name, type));
    else
        done = mCallStack.back()->back()->add(Nda::Symbol(name, type));

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
    Nda::Symbol symbol;
    if (!find(name,symbol))
        return Nda::Undefined;
    return symbol.type->dataType;
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
bool NdaState::bind(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, const std::shared_ptr<NadaParser::ASTNode> &block)
{
    assert(!name.empty());
    return mFunctions.bind(type.empty() ? name : BUILD_METHOD(type,name),parameters,block);
}

//-------------------------------------------------------------------------------------------------
bool NdaState::hasFunction(const std::string &type, const std::string &name, const NadaValues &parameters)
{
    return mFunctions.contains(type.empty() ? name : BUILD_METHOD(type,name),parameters);
}

//-------------------------------------------------------------------------------------------------
Nda::FunctionEntry &NdaState::function(const std::string &type, const std::string &name, const NadaValues &parameters)
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
bool NdaState::find(const std::string &symbolName, Nda::Symbol &symbol) const
{
    // First priority: current function scope:
    if (!mCallStack.empty()) {
        const auto& currentFrame = mCallStack.back();
        for (auto it = currentFrame->rbegin(); it != currentFrame->rend(); ++it) {
            if ((*it)->get(symbolName,symbol)) {
                return true;
            }
        }
    }

    // second: globals:
    for (auto it = mGlobals.rbegin(); it != mGlobals.rend(); ++it) {
        if ((*it)->get(symbolName,symbol)) {
            return true;
        }
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaState::value(const std::string &symbolName) const
{
    Nda::Symbol symbol;
    if (!find(symbolName,symbol))
        return NdaVariant();
    return *symbol.value;
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaState::valueRef(const std::string &symbolName)
{
    Nda::Symbol symbol;
    bool done = find(symbolName,symbol);
    assert(done);
    return *(symbol.value);
}

//-------------------------------------------------------------------------------------------------
NdaVariant *NdaState::valuePtr(const std::string &symbolName)
{
    Nda::Symbol symbol;
    bool done = find(symbolName,symbol);
    assert(done);
    return symbol.value;
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
void NdaState::destroy()
{
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
void NdaState::onWith(WithCallback cb)
{
   mWithCallback = std::move(cb);
}

//-------------------------------------------------------------------------------------------------
void NdaState::requestAddon(std::string name)
{
    if (!mWithCallback)
        return;
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

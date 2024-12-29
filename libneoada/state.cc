#include "private/utils.h"
#include "state.h"
#include "value.h"
#include <cassert>

#define BUILD_METHOD(t,n) (t + ":" + n)

//-------------------------------------------------------------------------------------------------
NadaState::NadaState()
{
    reset();
}

//-------------------------------------------------------------------------------------------------
NadaState::~NadaState()
{
}

void NadaState::reset()
{
    mGlobals.clear();
    mCallStack.clear();
    mGlobals.push_back(new NadaSymbolTable(NadaSymbolTable::GlobalScope));
}

//-------------------------------------------------------------------------------------------------
bool NadaState::define(const std::string &name, const std::string &typeName, bool isVolatile)
{
    Nda::Type t = Nda::typeByString(typeName);
    if (t == Nda::Undefined)
        return false;

    bool done;
    if (mCallStack.empty())
        done = mGlobals.back()->add(Nda::Symbol(t,Nda::toLower(name), typeName));
    else
        done = mCallStack.back()->back()->add(Nda::Symbol(t,Nda::toLower(name), typeName));

    if (done && isVolatile) {
        NadaValue &value = valueRef(name);
        if (mVolatileCtor)
            mVolatileCtor(name,value);
    }

    return done;
}

//-------------------------------------------------------------------------------------------------
Nda::Type NadaState::typeOf(const std::string &name) const
{
    Nda::Symbol symbol;
    if (!find(name,symbol))
        return Nda::Undefined;
    return symbol.type;
}

//-------------------------------------------------------------------------------------------------
bool NadaState::bind(const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb)
{
    assert(!name.empty());
    return mFunctions.bind(name,parameters,cb);
}

//-------------------------------------------------------------------------------------------------
bool NadaState::bind(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, const std::shared_ptr<NadaParser::ASTNode> &block)
{
    assert(!name.empty());
    return mFunctions.bind(type.empty() ? name : BUILD_METHOD(type,name),parameters,block);
}

//-------------------------------------------------------------------------------------------------
bool NadaState::hasFunction(const std::string &type, const std::string &name, const NadaValues &parameters)
{
    return mFunctions.contains(type.empty() ? name : BUILD_METHOD(type,name),parameters);
}

//-------------------------------------------------------------------------------------------------
Nda::FunctionEntry &NadaState::function(const std::string &type, const std::string &name, const NadaValues &parameters)
{
    return mFunctions.symbol(type.empty() ? name : BUILD_METHOD(type,name),parameters);
}

//-------------------------------------------------------------------------------------------------
bool NadaState::bind(const std::string &type, const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb)
{
    assert(!type.empty());
    assert(!name.empty());
    return bind(BUILD_METHOD(type,name), parameters, std::move(cb));
}

//-------------------------------------------------------------------------------------------------
bool NadaState::find(const std::string &symbolName, Nda::Symbol &symbol) const
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
NadaValue NadaState::value(const std::string &symbolName) const
{
    Nda::Symbol symbol;
    if (!find(symbolName,symbol))
        return NadaValue();
    return *symbol.value;
}

//-------------------------------------------------------------------------------------------------
NadaValue &NadaState::valueRef(const std::string &symbolName)
{
    Nda::Symbol symbol;
    bool done = find(symbolName,symbol);
    assert(done);
    return *(symbol.value);
}

//-------------------------------------------------------------------------------------------------
NadaValue *NadaState::valuePtr(const std::string &symbolName)
{
    Nda::Symbol symbol;
    bool done = find(symbolName,symbol);
    assert(done);
    return symbol.value;
}

//-------------------------------------------------------------------------------------------------
void NadaState::pushStack(NadaSymbolTable::Scope s)
//                            enter Function/Procedure/Method
{
    mCallStack.push_back(new NadaSymbolTables());
    mCallStack.back()->push_back(new NadaSymbolTable(s)); // TODO: Performance.. maybe only on new local symbol?
}

//-------------------------------------------------------------------------------------------------
void NadaState::popStack()
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
bool NadaState::inLoopScope() const
{
    if (!mCallStack.empty())
        return inLoopScope(*mCallStack.back());
    return inLoopScope(mGlobals);
}

//-------------------------------------------------------------------------------------------------
bool NadaState::inLoopScope(const NadaSymbolTables &tables) const
{
    for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
        if ((*it) && (*it)->scope() == NadaSymbolTable::LoopScope)
            return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
void NadaState::onVolatileCtor(NadaState::CtorCallback cb)
{
    mVolatileCtor = std::move(cb);
}

//-------------------------------------------------------------------------------------------------
void NadaState::pushScope(NadaSymbolTable::Scope s)
//                            enter block (if/while/...)
{
    if (mCallStack.empty())
        mGlobals.push_back(new NadaSymbolTable(s));
    else
        mCallStack.back()->push_back(new NadaSymbolTable(s));
}

//-------------------------------------------------------------------------------------------------
void NadaState::popScope()
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

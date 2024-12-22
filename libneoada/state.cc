#include "utils.h"
#include "state.h"
#include "value.h"
#include <cassert>

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
bool NadaState::define(const std::string &name, const std::string &typeName)
{
    Nada::Type t = Nada::typeByString(typeName);
    if (t == Nada::Undefined)
        return false;

    if (mCallStack.empty())
        return mGlobals.back()->add(NadaSymbol(t,Nada::toLower(name)));
    return mCallStack.back()->back()->add(NadaSymbol(t,Nada::toLower(name)));
}

//-------------------------------------------------------------------------------------------------
Nada::Type NadaState::typeOf(const std::string &name) const
{
    NadaSymbol symbol;
    if (!find(name,symbol))
        return Nada::Undefined;
    return symbol.type;
}

//-------------------------------------------------------------------------------------------------
bool NadaState::bind(const std::string &name, const NadaFncParameters &parameters, NadaFncCallback cb)
{
    return mFunctions.bind(name,parameters,cb);
}

//-------------------------------------------------------------------------------------------------
bool NadaState::bind(const std::string &name, const NadaFncParameters &parameters, const std::shared_ptr<NadaParser::ASTNode> &block)
{
    return mFunctions.bind(name,parameters,block);
}

//-------------------------------------------------------------------------------------------------
bool NadaState::hasFunction(const std::string &name, const NadaValues &parameters)
{
    return mFunctions.contains(name,parameters);
}

//-------------------------------------------------------------------------------------------------
NadaFunctionEntry &NadaState::function(const std::string &name, const NadaValues &parameters)
{
    return mFunctions.symbol(name,parameters);
}

//-------------------------------------------------------------------------------------------------
bool NadaState::find(const std::string &symbolName,NadaSymbol &symbol) const
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
    NadaSymbol symbol;
    if (!find(symbolName,symbol))
        return NadaValue();
    return *symbol.value;
}

//-------------------------------------------------------------------------------------------------
NadaValue &NadaState::valueRef(const std::string &symbolName)
{
    NadaSymbol symbol;
    bool done = find(symbolName,symbol);
    assert(done);
    return *(symbol.value);
}

//-------------------------------------------------------------------------------------------------
NadaValue *NadaState::valuePtr(const std::string &symbolName)
{
    NadaSymbol symbol;
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
        for (auto *table : (*frame)) {
            delete table;
        }
        delete frame;
        mCallStack.back()->pop_back();
    }
}

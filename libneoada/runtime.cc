#include <iostream>
#include <ostream>

#include "runtime.h"
#include "exception.h"
#include "state.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

#include "addons/AdaList.h"

//-------------------------------------------------------------------------------------------------
NdaRuntime::NdaRuntime()
    : mState(nullptr)
{
}

//-------------------------------------------------------------------------------------------------
NdaRuntime::~NdaRuntime()
{
    destroy();
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::reset()
{
    destroy();
    mState = new NdaState();
    mState->onWith([this](const std::string &addonName) {
        if (addonName == "ada.list")
            loadAddonAdaList();
    });
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaRuntime::runScript(const std::string &script, NdaException *exception)
{
    if (!mState)
        reset();

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NdaInterpreter  interpreter(mState);

    try {
        auto ast = parser.parse(script);
        return interpreter.execute(ast);
    } catch (NdaException &ex) {
        if (exception)
            *exception = ex;
        else
            std::cerr << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "NeoAda Fatal Runtime Error!" << std::endl;
    }

    return NdaVariant();

}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::loadAddonAdaList()
{
    if (!mState)
        reset();
    Nda::add_AdaList_symbols(mState);
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::destroy()
{
    if (mState) {
        delete mState;
        mState = nullptr;
    }
}
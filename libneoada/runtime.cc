#include <cassert>
#include <iostream>
#include <ostream>

#include "runtime.h"
#include "exception.h"
#include "state.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

#include "addons/AdaList.h"
#include "addons/AdaBytes.h"
#include "addons/AdaString.h"
#include "addons/AdaTextEncoding.h"
#include "addons/AdaIoFile.h"

//-------------------------------------------------------------------------------------------------
NdaRuntime::NdaRuntime()
    : mState(nullptr)
    , mInterpreter(nullptr)
{
    reset();
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
    mState       = new NdaState();
    mInterpreter = new NdaInterpreter(mState);
    mLastError.clear();

    mState->onWith([this](const std::string &addonName) {
        if (addonName == "ada.list")
            loadAddonAdaList();
        if (addonName == "ada.bytes")
            loadAddonAdaBytes();
        if (addonName == "ada.string")
            loadAddonAdaString();
        if (addonName == "ada.text.encoding")
            loadAddonAdaTextEncoding();
        if (addonName == "ada.io.file" || addonName == "ada.io")
            loadAddonAdaIoFile();
    });
}

//-------------------------------------------------------------------------------------------------
bool NdaRuntime::hasError() const
{
    return !mLastError.empty();
}

//-------------------------------------------------------------------------------------------------
std::string NdaRuntime::lastError() const
{
    return mLastError;
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaRuntime::runScript(const std::string &script, NdaException *exception)
{
    if (!mState)
        reset();

    NdaLexer       lexer;
    NdaParser      parser(lexer);

    mLastError.clear();
    try {
        auto ast = parser.parse(script);
        return mInterpreter->execute(ast);
    } catch (NdaException &ex) {
        mLastError = ex.what();
        if (exception)
            *exception = ex;
        else
            std::cerr << ex.what() << std::endl;
    } catch (const std::exception& ex) {
        mLastError = ex.what();
        std::cerr << "NeoAda Fatal Runtime Error: " << ex.what() << std::endl;
    } catch (...) {
        mLastError = "NeoAda Unknown Fatal Runtime Error";
        std::cerr << "NeoAda Fatal Runtime Error!" << std::endl;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaState *NdaRuntime::state()
{
    return mState;
}


//-------------------------------------------------------------------------------------------------
NdaValue NdaRuntime::invokeFnc(const std::string &fncName)
{
    assert(mInterpreter);

    NdaVariants args;

    mInterpreter->invokeFnc("", fncName, args);

    return mState->toValue(mState->ret());
}

//-------------------------------------------------------------------------------------------------
NdaValue NdaRuntime::invokeFnc(const std::string &fncName, const NdaValue &arg1)
{
    assert(mInterpreter);

    NdaVariants args;
    args.push_back(mState->toVariant(arg1));

    mInterpreter->invokeFnc("", fncName, args);
    return mState->toValue(mState->ret());
}

//-------------------------------------------------------------------------------------------------
NdaValue NdaRuntime::invokeFnc(const std::string &fncName, const NdaValue &arg1, const NdaValue &arg2)
{
    assert(mInterpreter);

    NdaVariants args;
    args.push_back(mState->toVariant(arg1));
    args.push_back(mState->toVariant(arg2));

    mInterpreter->invokeFnc("", fncName, args);

    return mState->toValue(mState->ret());
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::loadAddonAdaString()
{
    if (!mState)
        reset();
    Nda::add_AdaString_symbols(mState);
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::loadAddonAdaList()
{
    if (!mState)
        reset();
    Nda::add_AdaList_symbols(mState);
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::loadAddonAdaBytes()
{
    if (!mState)
        reset();
    Nda::add_AdaBytes_symbols(mState);
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::loadAddonAdaIoFile()
{
    if (!mState)
        reset();
    Nda::add_AdaIoFile_symbols(mState);
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::loadAddonAdaTextEncoding()
{
    if (!mState)
        reset();
    Nda::add_AdaTextEncoding_symbols(mState);
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::destroy()
{
    if (mInterpreter) {
        delete mInterpreter;
        mInterpreter = nullptr;
    }

    if (mState) {
        delete mState;
        mState = nullptr;
    }
}

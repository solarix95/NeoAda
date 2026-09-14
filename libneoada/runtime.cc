#include <cassert>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>

#include "runtime.h"
#include "exception.h"
#include "state.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

#include "addons/AdaList.h"
#include "addons/AdaDict.h"
#include "addons/AdaBytes.h"
#include "addons/AdaString.h"
#include "addons/AdaMath.h"
#include "addons/AdaTextEncoding.h"
#include "addons/AdaIoFile.h"
#include "addons/AdaDateTime.h"
#include "addons/AdaRegexp.h"
#include "addons/AdaJson.h"

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
        if (addonName == "ada.dict")
            loadAddonAdaDict();
        if (addonName == "ada.bytes")
            loadAddonAdaBytes();
        if (addonName == "ada.string")
            loadAddonAdaString();
        if (addonName == "ada.math")
            loadAddonAdaMath();
        if (addonName == "ada.text.encoding")
            loadAddonAdaTextEncoding();
        if (addonName == "ada.io.file" || addonName == "ada.io")
            loadAddonAdaIoFile();
        if (addonName == "ada.datetime" || addonName == "ada.date.time" || addonName == "ada.date")
            loadAddonAdaDateTime();
        if (addonName == "ada.regexp" || addonName == "ada.regex")
            loadAddonAdaRegexp();
        if (addonName == "ada.json")
            loadAddonAdaJson();
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
    return runScript(script, "", exception);
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaRuntime::runScript(const std::string &script, const std::string &sourceName, NdaException *exception)
{
    if (!mState)
        reset();

    NdaLexer       lexer;
    NdaParser      parser(lexer);

    mLastError.clear();
    try {
        auto ast = parser.parse(script);
        const uint32_t sourceId = mState->registerSource(sourceName, script, "script");
        return mInterpreter->execute(ast, nullptr, sourceId);
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
NdaVariant NdaRuntime::evaluateFormula(const std::string &formula, NdaException *exception)
{
    return evaluateFormula(formula, "", exception);
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaRuntime::evaluateFormula(const std::string &formula, const std::string &sourceName, NdaException *exception)
{
    NdaFormula prepared = prepareFormula(formula, sourceName, exception);
    if (!prepared.isValid())
        return NdaVariant();
    return executeFormula(prepared, exception);
}

//-------------------------------------------------------------------------------------------------
NdaFormula NdaRuntime::prepareFormula(const std::string &formula, NdaException *exception)
{
    return prepareFormula(formula, "", exception);
}

//-------------------------------------------------------------------------------------------------
NdaFormula NdaRuntime::prepareFormula(const std::string &formula, const std::string &sourceName, NdaException *exception)
{
    if (!mState)
        reset();

    NdaLexer       lexer;
    NdaParser      parser(lexer);

    mLastError.clear();
    try {
        auto ast = parser.parseFormula(formula);
        if (!mInterpreter->isFormula(ast))
            throw NdaException(Nada::Error::InvalidStatement,0,0);
        const uint32_t sourceId = mState->registerSource(sourceName, formula, "formula");
        return NdaFormula(mInterpreter->prepare(ast, sourceId));
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

    return NdaFormula();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaRuntime::executeFormula(NdaFormula &formula, NdaException *exception)
{
    if (!mState)
        reset();

    if (!formula.isValid())
        return NdaVariant();

    mLastError.clear();
    try {
        auto ret = mInterpreter->executeFormula(formula.runnable(), mState);
        ret.dereference();
        return ret;
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
NdaVariant NdaRuntime::runFile(const std::string &fileName, NdaException *exception)
{
    std::ifstream file(fileName);
    if (!file) {
        NdaException ex(Nada::Error::InvalidStatement, 0, 0, fileName);
        mLastError = ex.what();
        if (exception)
            *exception = ex;
        return NdaVariant();
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return runScript(contents.str(), fileName, exception);
}

//-------------------------------------------------------------------------------------------------
NdaState *NdaRuntime::state()
{
    return mState;
}

//-------------------------------------------------------------------------------------------------
std::vector<std::string> NdaRuntime::globalFunctions() const
{
    if (!mState)
        return std::vector<std::string>();
    return mState->globalFunctions();
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

    if (mInterpreter->invokeFnc("", fncName, args) == Nada::NoError && mState->ret().type() != Nda::Undefined)
        return mState->toValue(mState->ret());
    return NdaValue();
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
void NdaRuntime::invokePrc(const std::string &prcName)
{
    assert(mInterpreter);

    mLastError.clear();
    try {
        NdaVariants args;
        mInterpreter->invokeFnc("", prcName, args);
    } catch (NdaException &ex) {
        mLastError = ex.what();
        std::cerr << ex.what() << std::endl;
    } catch (const std::exception &ex) {
        mLastError = ex.what();
        std::cerr << "NeoAda Fatal Runtime Error: " << ex.what() << std::endl;
    } catch (...) {
        mLastError = "NeoAda Unknown Fatal Runtime Error";
        std::cerr << "NeoAda Fatal Runtime Error!" << std::endl;
    }
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::invokePrc(const std::string &prcName, const NdaValue &arg1)
{
    assert(mInterpreter);

    mLastError.clear();
    try {
        NdaVariants args;
        args.push_back(mState->toVariant(arg1));
        mInterpreter->invokeFnc("", prcName, args);
    } catch (NdaException &ex) {
        mLastError = ex.what();
        std::cerr << ex.what() << std::endl;
    } catch (const std::exception &ex) {
        mLastError = ex.what();
        std::cerr << "NeoAda Fatal Runtime Error: " << ex.what() << std::endl;
    } catch (...) {
        mLastError = "NeoAda Unknown Fatal Runtime Error";
        std::cerr << "NeoAda Fatal Runtime Error!" << std::endl;
    }
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::invokePrc(const std::string &prcName, const NdaValue &arg1, const NdaValue &arg2,
                           const NdaValue &arg3, const NdaValue &arg4)
{
    assert(mInterpreter);

    mLastError.clear();
    try {
        NdaVariants args;
        args.push_back(mState->toVariant(arg1));
        args.push_back(mState->toVariant(arg2));
        args.push_back(mState->toVariant(arg3));
        args.push_back(mState->toVariant(arg4));
        mInterpreter->invokeFnc("", prcName, args);
    } catch (NdaException &ex) {
        mLastError = ex.what();
        std::cerr << ex.what() << std::endl;
    } catch (const std::exception &ex) {
        mLastError = ex.what();
        std::cerr << "NeoAda Fatal Runtime Error: " << ex.what() << std::endl;
    } catch (...) {
        mLastError = "NeoAda Unknown Fatal Runtime Error";
        std::cerr << "NeoAda Fatal Runtime Error!" << std::endl;
    }
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::invokePrc(const std::string &prcName, const NdaValue &arg1, const NdaValue &arg2,
                           const NdaValue &arg3, const NdaValue &arg4, const NdaValue &arg5)
{
    assert(mInterpreter);

    mLastError.clear();
    try {
        NdaVariants args;
        args.push_back(mState->toVariant(arg1));
        args.push_back(mState->toVariant(arg2));
        args.push_back(mState->toVariant(arg3));
        args.push_back(mState->toVariant(arg4));
        args.push_back(mState->toVariant(arg5));
        mInterpreter->invokeFnc("", prcName, args);
    } catch (NdaException &ex) {
        mLastError = ex.what();
        std::cerr << ex.what() << std::endl;
    } catch (const std::exception &ex) {
        mLastError = ex.what();
        std::cerr << "NeoAda Fatal Runtime Error: " << ex.what() << std::endl;
    } catch (...) {
        mLastError = "NeoAda Unknown Fatal Runtime Error";
        std::cerr << "NeoAda Fatal Runtime Error!" << std::endl;
    }
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
void NdaRuntime::loadAddonAdaDict()
{
    if (!mState)
        reset();
    Nda::add_AdaDict_symbols(mState);
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::loadAddonAdaBytes()
{
    if (!mState)
        reset();
    Nda::add_AdaBytes_symbols(mState);
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::loadAddonAdaMath()
{
    if (!mState)
        reset();
    Nda::add_AdaMath_symbols(mState);
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
void NdaRuntime::loadAddonAdaDateTime()
{
    if (!mState)
        reset();
    Nda::add_AdaDateTime_symbols(mState);
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::loadAddonAdaRegexp()
{
    if (!mState)
        reset();
    Nda::add_AdaRegexp_symbols(mState);
}

//-------------------------------------------------------------------------------------------------
void NdaRuntime::loadAddonAdaJson()
{
    if (!mState)
        reset();
    loadAddonAdaIoFile();
    Nda::add_AdaJson_symbols(mState);
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

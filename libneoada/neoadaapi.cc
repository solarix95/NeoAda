
#include <iostream>
#include "neoadaapi.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "exception.h"

namespace NeoAda
{

//-------------------------------------------------------------------------------------------------
NdaVariant evaluate(const std::string &shortScript, NdaState &state, Exception *exception)
{
    return evaluate(shortScript, state, "", exception);
}

//-------------------------------------------------------------------------------------------------
NdaVariant evaluate(const std::string &shortScript, NdaState &state, const std::string &sourceName, Exception *exception)
{
    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaInterpreter interpreter(&state);

    state.reset();
    try {
        auto ast = parser.parse(shortScript);
        const uint32_t sourceId = state.registerSource(sourceName, shortScript, "script");
        auto ret = interpreter.execute(ast, nullptr, sourceId);
        ret.dereference();
        return ret;
    } catch (NdaException &ex) {
        if (exception)
            *exception = ex;
        else
            std::cerr << ex.what() << std::endl;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaVariant evaluateFormula(const std::string &formula, NdaState &state, Exception *exception)
{
    return evaluateFormula(formula, state, "", exception);
}

//-------------------------------------------------------------------------------------------------
NdaVariant evaluateFormula(const std::string &formula, NdaState &state, const std::string &sourceName, Exception *exception)
{
    auto prepared = prepareFormula(formula, state, sourceName, exception);
    if (!prepared.isValid())
        return NdaVariant();
    return executeFormula(prepared, state, exception);
}

//-------------------------------------------------------------------------------------------------
NdaFormula prepareFormula(const std::string &formula, NdaState &state, Exception *exception)
{
    return prepareFormula(formula, state, "", exception);
}

//-------------------------------------------------------------------------------------------------
NdaFormula prepareFormula(const std::string &formula, NdaState &state, const std::string &sourceName, Exception *exception)
{
    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaInterpreter interpreter(&state);

    try {
        auto ast = parser.parseFormula(formula);
        if (!interpreter.isFormula(ast))
            throw NdaException(Nada::Error::InvalidStatement,0,0);
        const uint32_t sourceId = state.registerSource(sourceName, formula, "formula");
        return NdaFormula(interpreter.prepare(ast, sourceId));
    } catch (NdaException &ex) {
        if (exception)
            *exception = ex;
        else
            std::cerr << ex.what() << std::endl;
    }

    return NdaFormula();
}

//-------------------------------------------------------------------------------------------------
NdaVariant executeFormula(NdaFormula &formula, NdaState &state, Exception *exception)
{
    if (!formula.isValid())
        return NdaVariant();

    NdaInterpreter interpreter(&state);

    try {
        auto ret = interpreter.executeFormula(formula.runnable(), &state);
        ret.dereference();
        return ret;
    } catch (NdaException &ex) {
        if (exception)
            *exception = ex;
        else
            std::cerr << ex.what() << std::endl;
    }

    return NdaVariant();
}

}


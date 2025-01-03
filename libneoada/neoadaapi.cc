
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
    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaInterpreter interpreter(&state);

    state.reset();
    try {
        auto ast = parser.parse(shortScript);
        auto ret = interpreter.execute(ast);
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



#include <iostream>
#include "neoadaapi.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "exception.h"

namespace NeoAda
{

//-------------------------------------------------------------------------------------------------
NadaValue evaluate(const std::string &shortScript, Exception *exception)
{
    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    try {
        auto ast = parser.parse(shortScript);
        return interpreter.execute(ast);
    } catch (NadaException &ex) {
        if (exception)
            *exception = ex;
        else
            std::cerr << ex.what() << std::endl;
    }

    return NadaValue();
}

}


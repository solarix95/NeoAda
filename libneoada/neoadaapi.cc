
#include "neoadaapi.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

namespace NeoAda
{

//-------------------------------------------------------------------------------------------------
NadaValue evaluate(const std::string &shortScript)
{
    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(shortScript);
    return interpreter.execute(ast);
}

}


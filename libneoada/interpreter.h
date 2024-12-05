#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include "value.h"
#include "state.h"

class NadaInterpreter
{
public:
    NadaInterpreter(NadaState *state);

    NadaValue execute(const std::shared_ptr<NadaParser::ASTNode> &node, NadaState *state = nullptr);

private:
    NadaValue  executeState(const std::shared_ptr<NadaParser::ASTNode> &node, NadaState *state);

    NadaState *mState;
};

#endif // INTERPRETER_H

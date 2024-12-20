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
    NadaValue  &executeState(const std::shared_ptr<NadaParser::ASTNode> &node, NadaState *state);
    NadaValue  &executeForLoopRange(const std::shared_ptr<NadaParser::ASTNode> &node, NadaState *state);
    NadaValue  &evaluateBinaryOperator(const std::shared_ptr<NadaParser::ASTNode> &node, NadaState *state);
    NadaValue  &evaluateUnaryOperator(const std::shared_ptr<NadaParser::ASTNode> &node, NadaState *state);

    enum ExecState {
        RunState,
        ReturnState,
        BreakState,
        ContinueState
    };

    ExecState  mExecState;
    NadaState *mState;
};

#endif // INTERPRETER_H

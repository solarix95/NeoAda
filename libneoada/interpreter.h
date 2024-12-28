#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include "value.h"
#include "state.h"

/*
    NadaInterpreter: main NeoAda Engine.

    Executing of a NeoAda-AST on a NeoAda-State.
*/

class NadaInterpreter
{
public:
    NadaInterpreter(NadaState *state);

    NadaValue execute(const NadaParser::ASTNodePtr &node, NadaState *state = nullptr);

private:
    NadaValue  &executeState(const NadaParser::ASTNodePtr &node, NadaState *state);
    NadaValue  &executeForLoopRange(const NadaParser::ASTNodePtr &node, NadaState *state);
    NadaValue  &evaluateBinaryOperator(const NadaParser::ASTNodePtr &node, NadaState *state);
    NadaValue  &evaluateUnaryOperator(const NadaParser::ASTNodePtr &node, NadaState *state);
    NadaValue  &executeFunctionCall(const NadaParser::ASTNodePtr &node, NadaState *state);
    void        defineProcedure(const NadaParser::ASTNodePtr &node, NadaState *state);
    void        defineFunction(const NadaParser::ASTNodePtr &node, NadaState *state);

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

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include "variant.h"
#include "state.h"

/*
    NadaInterpreter: main NeoAda Engine.

    Executing of a NeoAda-AST on a NeoAda-State.
*/

class NdaInterpreter
{
public:
    NdaInterpreter(NdaState *state);

    NdaVariant execute(const NadaParser::ASTNodePtr &node, NdaState *state = nullptr);

private:
    NdaVariant  &executeState(const NadaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &executeForLoopRange(const NadaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &evaluateBinaryOperator(const NadaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &evaluateUnaryOperator(const NadaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &executeFunctionCall(const NadaParser::ASTNodePtr &node, NdaState *state);
    void        defineProcedure(const NadaParser::ASTNodePtr &node, NdaState *state);
    void        defineFunction(const NadaParser::ASTNodePtr &node, NdaState *state);

    enum ExecState {
        RunState,
        ReturnState,
        BreakState,
        ContinueState
    };

    ExecState  mExecState;
    NdaState *mState;
};

#endif // INTERPRETER_H

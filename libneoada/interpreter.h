#ifndef LIB_NEOADA_INTERPRETER_H
#define LIB_NEOADA_INTERPRETER_H

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

    NdaVariant execute(const NdaParser::ASTNodePtr &node, NdaState *state = nullptr);

private:
    NdaVariant  &executeState(const NdaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &executeNumber(const NdaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &executeForLoopRange(const NdaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &evaluateBinaryOperator(const NdaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &evaluateUnaryOperator(const NdaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &executeFunctionCall(const NdaParser::ASTNodePtr &node, NdaState *state);
    void        defineProcedure(const NdaParser::ASTNodePtr &node, NdaState *state);
    void        defineFunction(const NdaParser::ASTNodePtr &node, NdaState *state);

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

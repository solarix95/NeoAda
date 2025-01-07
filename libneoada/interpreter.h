#ifndef LIB_NEOADA_INTERPRETER_H
#define LIB_NEOADA_INTERPRETER_H

#include "parser.h"
#include "variant.h"
#include "state.h"
#include "private/runnable.h"

/*
    NadaInterpreter: main NeoAda Engine.

    Executing of a NeoAda-AST on a NeoAda-State.
*/

class NdaInterpreter
{
public:
    NdaInterpreter(NdaState *state);

    NdaVariant execute(const NdaParser::ASTNodePtr &node, NdaState *state = nullptr);
    NdaVariant execute(Nda::Runnable *node, NdaState *state = nullptr);

    Nda::Runnable *prepare(const NdaParser::ASTNodePtr &node);

private:
    enum ExecState {
        RunState,
        ReturnState,
        BreakState,
        ContinueState
    };

    NdaVariant  &executeState(const NdaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &executeNumber(const NdaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &executeForLoopRange(const NdaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &evaluateBinaryOperator(const NdaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &evaluateUnaryOperator(const NdaParser::ASTNodePtr &node, NdaState *state);
    NdaVariant  &executeFunctionCall(const NdaParser::ASTNodePtr &node, NdaState *state);
    void        defineProcedure(const NdaParser::ASTNodePtr &node, NdaState *state);
    void        defineFunction(const NdaParser::ASTNodePtr &node, NdaState *state);

    void run(Nda::Runnable *node);
    void runProgramm(Nda::Runnable *node);
    void runLoopBlock(Nda::Runnable *node);
    void runSingleBlock(Nda::Runnable *node);
    void runDeclaration(Nda::Runnable *node);
    void runVolatileDeclaration(Nda::Runnable *node);
    void runAssignment(Nda::Runnable *node);
    void runFunctionCall(Nda::Runnable *node);
    void runStaticMethodCall(Nda::Runnable *node);
    void runInstanceMethodCall(Nda::Runnable *node);
    void runReturn(Nda::Runnable *node);
    void runBreak(Nda::Runnable *node);
    void runContinue(Nda::Runnable *node);
    void runIfStatement(Nda::Runnable *node);
    void runWhileLoop(Nda::Runnable *node);
    void runForLoopRange(Nda::Runnable *node);
    void runSubStatement(Nda::Runnable *node);

    void runBinaryEqual(Nda::Runnable *node);      // "="
    void runBinaryNotEqual(Nda::Runnable *node);   // "<>"
    void runBinaryGtThen(Nda::Runnable *node);     // ">"
    void runBinaryLtThen(Nda::Runnable *node);     // "<"
    void runBinaryEqGtThen(Nda::Runnable *node);   // ">="
    void runBinaryEqLtThen(Nda::Runnable *node);   // ">="
    void runBinaryConcat(Nda::Runnable *node);     // "&"
    void runBinaryMod(Nda::Runnable *node);        // "mod"
    void runBinaryPlus(Nda::Runnable *node);       // "+"
    void runBinaryMinus(Nda::Runnable *node);      // "-"
    void runBinaryMultiply(Nda::Runnable *node);   // "*"
    void runBinaryDivide(Nda::Runnable *node);     // "/"
    void runBinaryAnd(Nda::Runnable *node);        // "and"
    void runBinaryOr(Nda::Runnable *node);         // "or"
    void runBinaryXor(Nda::Runnable *node);        // "xor"

    void runUnaryMinus(Nda::Runnable *node);       // -x
    void runLengthOperator(Nda::Runnable *node);   // #x
    void runAccessOperator(Nda::Runnable *node);   // x[]

    void runLoadAddon(Nda::Runnable *node);
    void runDefineInstanceProcedure(Nda::Runnable *node);
    void runDefineSingleProcedure(Nda::Runnable *node);
    void runDefineInstanceFunction(Nda::Runnable *node);
    void runDefineSingleFunction(Nda::Runnable *node);
    void runLoopAbort(Nda::Runnable *node, ExecState nextState);

    void evalNumber(Nda::Runnable *node);
    void evalBoolean(Nda::Runnable *node);
    void evalListLiteral(Nda::Runnable *node);

    ExecState  mExecState;
    NdaState *mState;
};

#endif // INTERPRETER_H

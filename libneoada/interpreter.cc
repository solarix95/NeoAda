#include <assert.h>
#include "interpreter.h"

//-------------------------------------------------------------------------------------------------
NadaInterpreter::NadaInterpreter(NadaState *state)
    : mState(state)
{
}

//-------------------------------------------------------------------------------------------------
NadaValue NadaInterpreter::execute(const std::shared_ptr<NadaParser::ASTNode> &node, NadaState *state)
{
    assert(node);
    if (!state && !mState)
        return NadaValue();
    return executeState(node, state ? state : mState);
}

//-------------------------------------------------------------------------------------------------
NadaValue NadaInterpreter::executeState(const std::shared_ptr<NadaParser::ASTNode> &node, NadaState *state)
{
    assert(state);
    assert(node);

    NadaValue ret;

    switch (node->type) {
    case NadaParser::ASTNodeType::Program:
        for (auto &child : node->children)
            executeState(child, state);
        break;
    case NadaParser::ASTNodeType::Declaration:
        assert(node->children.size() >= 1);
        state->declareGlobal(node->value, node->children[0]->value);
        break;
    case NadaParser::ASTNodeType::FunctionCall: {
        NadaValues values;
        for (const auto &node : node->children) {
            values.push_back(executeState(node, state));
        }

        if (!state->hasFunction(node->value,values))
            return NadaValue(); // TODO: ERROR

        auto &fnc = state->function(node->value,values);

        return fnc.nativeCallback(NadaFncParameters());
    }   break;
    default:
        break;
    }
    return ret;

}

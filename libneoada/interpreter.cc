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
    case NadaParser::ASTNodeType::IfStatement: {
        assert(node->children.size() >= 1);
        NadaValue condition = executeState(node->children[0], state);
    } break;
    case NadaParser::ASTNodeType::FunctionCall: {

        NadaValues values;
        for (const auto &node : node->children) {
            values.push_back(executeState(node, state));
        }

        if (!state->hasFunction(node->value,values))
            return NadaValue(); // TODO: ERROR

        auto &fnc = state->function(node->value,values);

        return fnc.nativeCallback(fnc.fncValues(values));
    }   break;

    case NadaParser::ASTNodeType::Literal:
        ret.fromString(node->value);
        break;
    case NadaParser::ASTNodeType::Number: {
        auto done = ret.fromNumber(node->value);
        // FIXME: Error-Handling? done?
    }   break;
    case NadaParser::ASTNodeType::BinaryOperator: {
        return evaluateBinaryOperator(node, state);
    }   break;
    case NadaParser::ASTNodeType::Identifier: {
        return state->value(node->value);
    }   break;
    default:
        break;
    }
    return ret;

}

//-------------------------------------------------------------------------------------------------
NadaValue NadaInterpreter::evaluateBinaryOperator(const std::shared_ptr<NadaParser::ASTNode> &node, NadaState *state)
{
    NadaValue ret;

    if (node->value == ">") {
        assert(node->children.size() == 2);
        auto left  = executeState(node->children[0],state);
        auto right = executeState(node->children[1],state);

        bool done;
        bool result = left.greaterThen(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
    }

    return ret;


}

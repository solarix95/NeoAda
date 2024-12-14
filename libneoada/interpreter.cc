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

    mExecState = RunState;
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
    case NadaParser::ASTNodeType::Block:
        for (auto &child : node->children) {
            ret = executeState(child, state);
            if (mExecState == ReturnState)
                return ret;
        }
        break;
    case NadaParser::ASTNodeType::Expression: // just "()"
        assert(node->children.size() == 1);
        return executeState(node->children[0],state);
    case NadaParser::ASTNodeType::Declaration:
        assert(node->children.size() >= 1);
        if (!state->define(node->value, node->children[0]->value))
            return NadaValue(); // FIXME: Runtime-Error
        if (node->children.size() == 2) {
            auto &value = state->valueRef(node->value);
            NadaValue initialValue;

            if (node->children[1]->type == NadaParser::ASTNodeType::Number)
                initialValue.fromNumber(node->children[1]->value);
            else if (node->children[1]->type == NadaParser::ASTNodeType::Literal)
                initialValue.fromString(node->children[1]->value);
            else assert(0 && "unsupported type");

            if (initialValue.type() != Nada::Undefined)
                value.assign(initialValue);
        }
        break;
    case NadaParser::ASTNodeType::IfStatement: {
        assert(node->children.size() >= 1);
        bool conditionValid;
        bool condition = executeState(node->children[0], state).toBool(&conditionValid);
        // if (!conditionValid) // TODO: runtime error
        if (condition) {
            return executeState(node->children[1],state);
        }
    } break;
    case NadaParser::ASTNodeType::Return: {
        assert(node->children.size() == 1);

        auto returnValue = executeState(node->children[0], state);
        mExecState = ReturnState;
        return returnValue;
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
    case NadaParser::ASTNodeType::BooleanLiteral:
        ret.fromBool(node->value == "true");
        break;
    case NadaParser::ASTNodeType::Number: {
        auto done = ret.fromNumber(node->value);
        // FIXME: Error-Handling? done?
    }   break;
    case NadaParser::ASTNodeType::BinaryOperator: {
        return evaluateBinaryOperator(node, state);
    }   break;
    case NadaParser::ASTNodeType::UnaryOperator: {
        return evaluateUnaryOperator(node, state);
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

    assert(node->children.size() == 2);
    auto left  = executeState(node->children[0],state);
    auto right = executeState(node->children[1],state);
    bool done;

    if (node->value == ">") {
        bool result = left.greaterThen(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
    }

    if (node->value == "=") {
        bool result = left.equal(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
    }

    if (node->value == "/=") {
        bool result = !left.equal(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
    }

    if (node->value == "&") {
        auto result = left.concat(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        return result;
    }

    assert(0 && "not yet implemented");
    return ret;
}

//-------------------------------------------------------------------------------------------------
NadaValue NadaInterpreter::evaluateUnaryOperator(const std::shared_ptr<NadaParser::ASTNode> &node, NadaState *state)
{
    assert(node->children.size() == 1);

    auto expressionResult  = executeState(node->children[0],state);
    bool done;
    expressionResult.unaryOperator(node->value,&done);

    // FIXME: runtime error
    // if (!done)
    return expressionResult;
}

#include <assert.h>
#include <iostream>
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
        for (auto &child : node->children) {
            ret = executeState(child, state);
            if (mExecState == ReturnState)
                break;
        }
        break;
    case NadaParser::ASTNodeType::Block:
        state->pushScope();
        for (auto &child : node->children) {
            ret = executeState(child, state);
            if (mExecState == ReturnState)
                break;
            if (mExecState == BreakState)
                break;
            if (mExecState == ContinueState)
                break;
        }
        state->popScope();
        break;
    case NadaParser::ASTNodeType::Expression: // just "()"
        assert(node->children.size() == 1);
        return executeState(node->children[0],state);
    case NadaParser::ASTNodeType::Declaration:
        assert(node->children.size() >= 1);
        if (!state->define(node->value.lowerValue, node->children[0]->value.lowerValue))
            return NadaValue(); // FIXME: Runtime-Error
        if (node->children.size() == 2) {
            auto &value = state->valueRef(node->value.lowerValue);
            NadaValue initialValue;

            if (node->children[1]->type == NadaParser::ASTNodeType::Number)
                initialValue.fromNumber(node->children[1]->value.lowerValue);
            else if (node->children[1]->type == NadaParser::ASTNodeType::Literal)
                initialValue.fromString(node->children[1]->value.displayValue);
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
        } else if (node->children.size() == 3) {
            return executeState(node->children[2],state);
        }
    } break;
    case NadaParser::ASTNodeType::Else: {
        assert(node->children.size() == 1);
        return executeState(node->children[0],state);
    } break;
    case NadaParser::ASTNodeType::WhileLoop: {
        assert(node->children.size() == 2);
        bool conditionValid;
        bool condition;
        do {
            condition = executeState(node->children[0], state).toBool(&conditionValid);
            if (condition)
                executeState(node->children[1],state);
            if (mExecState == BreakState) {
                mExecState = RunState;
                break;
            }
            if (mExecState == ContinueState) {
                mExecState = RunState;
            }
        } while (condition && conditionValid);
    } break;
    case NadaParser::ASTNodeType::Return: {
        assert(node->children.size() == 1);

        auto returnValue = executeState(node->children[0], state);
        mExecState = ReturnState;
        return returnValue;
    } break;
    case NadaParser::ASTNodeType::Break: {
        mExecState = BreakState;
        return NadaValue();
    } break;
    case NadaParser::ASTNodeType::Continue: {
        mExecState = ContinueState;
        return NadaValue();
    } break;
    case NadaParser::ASTNodeType::FunctionCall: {

        NadaValues values;
        for (const auto &node : node->children) {
            values.push_back(executeState(node, state));
        }

        if (!state->hasFunction(node->value.lowerValue,values))
            return NadaValue(); // TODO: ERROR

        auto &fnc = state->function(node->value.lowerValue,values);

        return fnc.nativeCallback(fnc.fncValues(values));
    }   break;
    case NadaParser::ASTNodeType::Assignment: {
        assert(node->children.size() == 1);

        if (state->typeOf(node->value.lowerValue) == Nada::Undefined) {
            std::cerr << node->value.displayValue << std::endl;
            state->typeOf(node->value.lowerValue);
            assert(0 && "runtime error");
        }
        auto &value = state->valueRef(node->value.lowerValue);
        auto newValue = executeState(node->children[0], state);

        value.assign(newValue);

        // FIXME: runtime error.. if (!value.assign(newValue))
    }   break;
    case NadaParser::ASTNodeType::Literal:
        ret.fromString(node->value.displayValue);
        break;
    case NadaParser::ASTNodeType::BooleanLiteral:
        ret.fromBool(node->value.lowerValue == "true");
        break;
    case NadaParser::ASTNodeType::Number: {
        auto done = ret.fromNumber(node->value.lowerValue);
        // FIXME: Error-Handling? done?
    }   break;
    case NadaParser::ASTNodeType::BinaryOperator: {
        return evaluateBinaryOperator(node, state);
    }   break;
    case NadaParser::ASTNodeType::UnaryOperator: {
        return evaluateUnaryOperator(node, state);
    }   break;
    case NadaParser::ASTNodeType::Identifier: {
        return state->value(node->value.lowerValue);
    }   break;
    default:
        assert(0 && "not yet implemented");
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

    if (node->value.lowerValue == ">") {
        bool result = left.greaterThen(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
    }

    if (node->value.lowerValue == "<") {
        bool result = left.lessThen(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
    }

    if (node->value.lowerValue == ">=") {
        bool result1 = left.greaterThen(right, &done);
        bool result2 = result1 || left.equal(right);

        if (result1 || result2) {
            ret.fromBool(result1);
            return ret;
        }
        if (!done) // FIXME: runtime error!
            return NadaValue();

        ret.fromBool(false);
        return ret;
    }

    if (node->value.lowerValue == "<=") {
        bool result1 = left.lessThen(right, &done);
        bool result2 = result1 || left.equal(right);

        if (result1 || result2) {
            ret.fromBool(result1 || result2);
            return ret;
        }
        if (!done) // FIXME: runtime error!
            return NadaValue();

        ret.fromBool(false);
        return ret;
    }

    if (node->value.lowerValue == "=") {
        bool result = left.equal(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
    }

    if (node->value.lowerValue == "/=") {
        bool result = !left.equal(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
    }

    if (node->value.lowerValue == "&") {
        auto result = left.concat(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        return result;
    }

    if (node->value.lowerValue == "-") {
        auto result = left.subtract(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        return result;
    }

    if (node->value.lowerValue == "+") {
        auto result = left.add(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        return result;
    }

    if (node->value.lowerValue == "*") {
        auto result = left.multiply(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        return result;
    }
    if (node->value.lowerValue == "/") {
        auto result = left.division(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        return result;
    }

    if (node->value.lowerValue == "mod") {
        auto result = left.modulo(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        return result;
    }

    if (node->value.lowerValue == "and") {
        bool result = left.logicalAnd(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
    }

    if (node->value.lowerValue == "or") {
        bool result = left.logicalOr(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
    }

    if (node->value.lowerValue == "xor") {
        bool result = left.logicalXor(right, &done);
        if (!done) // FIXME: runtime error!
            return NadaValue();
        ret.fromBool(result);
        return ret;
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
    expressionResult.unaryOperator(node->value.lowerValue,&done);

    // FIXME: runtime error
    // if (!done)
    return expressionResult;
}

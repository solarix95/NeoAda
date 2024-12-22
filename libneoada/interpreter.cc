#include <assert.h>
#include <iostream>
#include "interpreter.h"

//-------------------------------------------------------------------------------------------------
NadaInterpreter::NadaInterpreter(NadaState *state)
    : mState(state)
{
}

//-------------------------------------------------------------------------------------------------
NadaValue NadaInterpreter::execute(const NadaParser::ASTNodePtr &node, NadaState *state)
{
    assert(node);
    if (!state && !mState)
        return NadaValue();

    mExecState = RunState;
    executeState(node, state ? state : mState);

    return (state ? state : mState)->ret();
}

//-------------------------------------------------------------------------------------------------
NadaValue &NadaInterpreter::executeState(const NadaParser::ASTNodePtr &node, NadaState *state)
{
    assert(state);
    assert(node);

    switch (node->type) {
    case NadaParser::ASTNodeType::Program:
        for (const auto &child : node->children) {
            executeState(child, state);
            if (mExecState == ReturnState)
                break;
        }
        break;
    case NadaParser::ASTNodeType::Procedure:
        defineProcedure(node,state);
        break;
    case NadaParser::ASTNodeType::Function:
        defineFunction(node,state);
        break;
    case NadaParser::ASTNodeType::Block:
        state->pushScope(node->parent->type == NadaParser::ASTNodeType::WhileLoop ? NadaSymbolTable::LoopScope : NadaSymbolTable::ConditionalScope);
        for (const auto &child : node->children) {
            executeState(child, state);
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
        break;
    case NadaParser::ASTNodeType::Declaration:
        assert(node->children.size() >= 1);
        if (!state->define(node->value.lowerValue, node->children[0]->value.lowerValue)) {
            state->ret().reset();
            return state->ret(); // FIXME: Runtime-Error
        }
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
            executeState(node->children[0], state);
            condition = state->ret().toBool(&conditionValid);
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
    case NadaParser::ASTNodeType::ForLoop: {
        assert(node->children.size() == 2); // range + body
        if (node->children[0]->type == NadaParser::ASTNodeType::Range)
            executeForLoopRange(node,state);
        return state->ret();
    } break;
    case NadaParser::ASTNodeType::Return: {
        assert(node->children.size() == 1);
        executeState(node->children[0], state);
        mExecState = ReturnState;
        return state->ret();
    } break;
    case NadaParser::ASTNodeType::Break: {
        if (state->inLoopScope()) {
            if (node->children.size() == 1) { // when condition
                bool conditionValid;
                bool condition = executeState(node->children[0], state).toBool(&conditionValid);
                // if (!conditionValid) // TODO: runtime error
                if (!condition)
                    return state->ret();
            }
            mExecState = BreakState;
        } else
            std::cerr << "INVALID BREAK ignored!"; // FIXME: runtime error
        return state->ret();
    } break;
    case NadaParser::ASTNodeType::Continue: {
        if (state->inLoopScope()) {
            if (node->children.size() == 1) { // when condition
                bool conditionValid;
                bool condition = executeState(node->children[0], state).toBool(&conditionValid);
                // if (!conditionValid) // TODO: runtime error
                if (!condition)
                    return state->ret();
            }
            mExecState = ContinueState;
        } else
            std::cerr << "INVALID CONTINUE ignored!"; // FIXME: runtime error
        return state->ret();
    } break;
    case NadaParser::ASTNodeType::FunctionCall: {

        NadaValues values;
        for (const auto &node : node->children) {
            values.push_back(executeState(node, state));
        }

        if (!state->hasFunction(node->value.lowerValue,values)) {
            state->ret().reset();
            return state->ret(); // TODO: ERROR
        }

        auto &fnc = state->function(node->value.lowerValue,values);

        if (fnc.block) {
            state->pushStack(NadaSymbolTable::LocalScope);
            assert(values.size() == fnc.parameters.size());

            /*
                parameter: "x"  : "any"
                value:     "42" : Type = Natural

                Push to stack   : declare x : Natural := 42;
            */

            for (int i = 0; i< fnc.parameters.size(); i++) {
                state->define(fnc.parameters[i].first, fnc.parameters[i].second);
                // TODO: if !define -> runtime error!
                NadaValue &valueRef = state->valueRef(fnc.parameters[i].first);
                valueRef.assign(values[i]);
            }

            executeState(fnc.block, state);
            state->popStack();
        } else
            fnc.nativeCallback(fnc.fncValues(values));
    }   break;
    case NadaParser::ASTNodeType::Assignment: {
        assert(node->children.size() == 1);
        if (state->typeOf(node->value.lowerValue) == Nada::Undefined) {
            std::cerr << node->value.displayValue << std::endl;
            state->typeOf(node->value.lowerValue);
            assert(0 && "runtime error");
        }


        auto &value = state->valueRef(node->value.lowerValue);

        executeState(node->children[0], state);
        value.assign(state->ret());

        // FIXME: runtime error.. if (!value.assign(newValue))
    }   break;
    case NadaParser::ASTNodeType::Literal:
        state->ret().fromString(node->value.displayValue);
        break;
    case NadaParser::ASTNodeType::BooleanLiteral:
        state->ret().fromBool(node->value.lowerValue == "true");
        break;
    case NadaParser::ASTNodeType::Number: {
        auto done = state->ret().fromNumber(node->value.lowerValue);
        // FIXME: Error-Handling? done?
    }   break;
    case NadaParser::ASTNodeType::BinaryOperator: {
        return evaluateBinaryOperator(node, state);
    }   break;
    case NadaParser::ASTNodeType::UnaryOperator: {
        return evaluateUnaryOperator(node, state);
    }   break;
    case NadaParser::ASTNodeType::Identifier: {
        state->ret() = state->value(node->value.lowerValue);
    }   break;
    default:
        assert(0 && "not yet implemented");
        break;
    }

    return state->ret();
}

//-------------------------------------------------------------------------------------------------
NadaValue &NadaInterpreter::executeForLoopRange(const NadaParser::ASTNodePtr &node, NadaState *state)
{
    assert(node->children.size() == 2);

    std::string varName = node->value.displayValue;
    assert(varName.length() > 0);

    assert(node->children[0]->children.size() == 2); // from .. to

    int64_t from,to;

    // TODO: runtime error handling
    NadaValue::fromNumber(node->children[0]->children[0]->value.lowerValue,from);
    NadaValue::fromNumber(node->children[0]->children[1]->value.lowerValue,to);

    state->pushScope(NadaSymbolTable::LoopScope);
    state->define(varName,"Natural");
    auto &valueRef = state->valueRef(varName);
    for (int64_t i = from; i<=to; i++) {
        valueRef.fromNumber(i);
        executeState(node->children[1],state);

        if (mExecState == BreakState) {
            mExecState = RunState;
            break;
        }
        if (mExecState == ContinueState) {
            mExecState = RunState;
        }
    }

    state->popScope();

    /*
    bool conditionValid;
    bool condition;
    do {
        executeState(node->children[0], state);
        condition = state->ret().toBool(&conditionValid);
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

    */
    return state->ret();
}

//-------------------------------------------------------------------------------------------------
NadaValue &NadaInterpreter::evaluateBinaryOperator(const NadaParser::ASTNodePtr &node, NadaState *state)
{
    assert(node->children.size() == 2);
    // static NadaValue left;
    // static NadaValue right;

    auto left  = executeState(node->children[0],state);
    auto right = executeState(node->children[1],state);
    bool done;

    if (node->value.lowerValue == ">") {
        bool result = left.greaterThen(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == "<") {
        bool result = left.lessThen(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == ">=") {
        bool result1 = left.greaterThen(right, &done);
        bool result2 = result1 || left.equal(right);

        if (result1 || result2) {
            state->ret().fromBool(result1);
            return state->ret();
        }
        if (!done) // FIXME: runtime error!
            return state->ret();

        state->ret().fromBool(false);
        return state->ret();
    }

    if (node->value.lowerValue == "<=") {
        bool result1 = left.lessThen(right, &done);
        bool result2 = result1 || left.equal(right);

        if (result1 || result2) {
            state->ret().fromBool(result1 || result2);
            return state->ret();
        }
        if (!done) // FIXME: runtime error!
            return state->ret();

        state->ret().fromBool(false);
        return state->ret();
    }

    if (node->value.lowerValue == "=") {
        bool result = left.equal(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == "/=") {
        bool result = !left.equal(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == "&") {
        state->ret() = left.concat(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        return state->ret();
    }

    if (node->value.lowerValue == "-") {
        state->ret() = left.subtract(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        return state->ret();
    }

    if (node->value.lowerValue == "+") {
        state->ret() = left.add(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        return state->ret();
    }

    if (node->value.lowerValue == "*") {
        state->ret() = left.multiply(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        return state->ret();
    }
    if (node->value.lowerValue == "/") {
        state->ret() = left.division(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        return state->ret();
    }

    if (node->value.lowerValue == "mod") {
        state->ret() = left.modulo(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        return state->ret();
    }

    if (node->value.lowerValue == "and") {
        bool result = left.logicalAnd(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == "or") {
        bool result = left.logicalOr(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == "xor") {
        bool result = left.logicalXor(right, &done);
        if (!done) // FIXME: runtime error!
            return state->ret();
        state->ret().fromBool(result);
        return state->ret();
    }

    assert(0 && "not yet implemented");
    return state->ret();
}

//-------------------------------------------------------------------------------------------------
NadaValue &NadaInterpreter::evaluateUnaryOperator(const NadaParser::ASTNodePtr &node, NadaState *state)
{
    assert(node->children.size() == 1);

    bool done;
    executeState(node->children[0],state).unaryOperator(node->value.lowerValue,&done);

    // FIXME: runtime error
    // if (!done)
    return state->ret();
}

//-------------------------------------------------------------------------------------------------
void NadaInterpreter::defineProcedure(const NadaParser::ASTNodePtr &node, NadaState *state)
{
    assert(node->type == NadaParser::ASTNodeType::Procedure);
    assert(node->children.size() == 2);

    auto parameters = node->children[0];
    auto block      = node->children[1];

    assert(parameters->type == NadaParser::ASTNodeType::FormalParameters);
    assert(block->type      == NadaParser::ASTNodeType::Block);

    NadaFncParameters fncParameters;
    for (const auto &p : parameters->children) {
        assert(p->children.size() >= 1); // TODO: in/out, child # 2
        fncParameters.push_back(std::make_pair(p->value.lowerValue,p->children[0]->value.lowerValue));
    }

    state->bind(node->value.lowerValue,fncParameters,block);
}

//-------------------------------------------------------------------------------------------------
void NadaInterpreter::defineFunction(const NadaParser::ASTNodePtr &node, NadaState *state)
{
    assert(node->type == NadaParser::ASTNodeType::Function);
    assert(node->children.size() == 3);

    auto parameters = node->children[0];
    auto returntype = node->children[1];
    auto block      = node->children[2];

    assert(parameters->type == NadaParser::ASTNodeType::FormalParameters);
    assert(block->type      == NadaParser::ASTNodeType::Block);

    NadaFncParameters fncParameters;
    for (const auto &p : parameters->children) {
        assert(p->children.size() >= 1); // TODO: in/out, child # 2
        fncParameters.push_back(std::make_pair(p->value.lowerValue,p->children[0]->value.lowerValue));
    }

    state->bind(node->value.lowerValue,fncParameters,block);
}

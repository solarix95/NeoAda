#include <assert.h>
#include <iostream>
#include "interpreter.h"
#include "exception.h"

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
    case NadaParser::ASTNodeType::VolatileDeclaration:
    case NadaParser::ASTNodeType::Declaration:
        assert(node->children.size() >= 1);
        if (!state->define(node->value.lowerValue, node->children[0]->value.lowerValue, node->type == NadaParser::ASTNodeType::VolatileDeclaration)) {
            state->ret().reset();
            return state->ret(); // FIXME: Runtime-Error
        }
        if (node->children.size() == 2) {
            auto &value = state->valueRef(node->value.lowerValue);
            NadaValue initialValue;

            state->ret().reset();
            executeState(node->children[1],state);
            if (!value.assign(state->ret()))
                throw NadaException(Nada::Error::AssignmentError,node->line,node->column, node->value.displayValue);
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
        if (node->children.size() == 1)
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
        executeFunctionCall(node, state);
    }   break;
    case NadaParser::ASTNodeType::Assignment: {
        assert(node->children.size() == 2);

        executeState(node->children[0], state);

        if (state->ret().myType() != Nda::Reference)
            throw NadaException(Nada::Error::InvalidAssignment,node->line,node->column, node->value.displayValue);

        auto targetValue = state->ret();

        executeState(node->children[1], state);

        if (!targetValue.assign(state->ret()))
            throw NadaException(Nada::Error::AssignmentError,node->line,node->column, node->value.displayValue);
    }   break;
    case NadaParser::ASTNodeType::Literal:
        state->ret().fromString(node->value.displayValue);
        break;
    case NadaParser::ASTNodeType::ListLiteral: {
        NadaValue ret;
        ret.initType(Nda::List);
        for (const auto &child : node->children) {
            ret.appendToList(executeState(child, state));
        }
        state->ret() = ret;
    } break;
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
        auto *value = state->valuePtr(node->value.lowerValue);
        if (value)
            state->ret().fromReference(value);
        else
            state->ret().reset();
        // TODO: runtime error
    }   break;
    case NadaParser::ASTNodeType::StaticMethodCall: {
        executeFunctionCall(node, state);
    }   break;
    case NadaParser::ASTNodeType::InstanceMethodCall: {
        executeFunctionCall(node, state);
    }   break;
    case NadaParser::ASTNodeType::AccessOperator: {
        assert(node->children.size() == 2);
        assert(node->children[0]->type == NadaParser::ASTNodeType::Identifier);

        if (state->typeOf(node->children[0]->value.lowerValue) == Nda::Undefined)
            throw NadaException(Nada::Error::UnknownSymbol,node->line,node->column, node->value.displayValue);

        if (state->typeOf(node->children[0]->value.lowerValue) != Nda::List)
            throw NadaException(Nada::Error::InvalidContainerType,node->line,node->column, node->value.displayValue);

        auto &targetList = state->valueRef(node->children[0]->value.lowerValue);

        executeState(node->children[1], state);

        bool done = false;
        int64_t index = state->ret().toInt64(&done);
        if (!done)
           throw NadaException(Nada::Error::InvalidAccessValue,node->line,node->column, node->value.displayValue);

        if (index < 0)
            throw NadaException(Nada::Error::InvalidAccessValue,node->line,node->column, node->value.displayValue);

        if (index >= targetList.listSize())
            throw NadaException(Nada::Error::InvalidAccessValue,node->line,node->column, node->value.displayValue);

        auto &targetValue = targetList.writeAccess((int)index);
        state->ret().fromReference(&targetValue);
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
        if (!done)
            throw NadaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == "<") {
        bool result = left.lessThen(right, &done);
        if (!done)
            throw NadaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);
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
        if (!done)
            throw NadaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);

        state->ret().fromBool(false);
        return state->ret();
    }

    if (node->value.lowerValue == "<=") {
        bool result1 = left.lessThen(right, &done);
        if (!done)
            throw NadaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);

        bool result2 = result1 || left.equal(right, &done);
        if (!done)
            throw NadaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);

        if (result1 || result2) {
            state->ret().fromBool(result1 || result2);
            return state->ret();
        }


        state->ret().fromBool(false);
        return state->ret();
    }

    if (node->value.lowerValue == "=") {
        bool result = left.equal(right, &done);
        if (!done)
            throw NadaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == "<>") {
        bool result = !left.equal(right, &done);
        if (!done)
            throw NadaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == "&") {
        state->ret() = left.concat(right, &done);
        if (!done)
            throw NadaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }

    if (node->value.lowerValue == "-") {
        state->ret() = left.subtract(right, &done);
        if (!done)
            throw NadaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }

    if (node->value.lowerValue == "+") {
        state->ret() = left.add(right, &done);
        if (!done)
            throw NadaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }

    if (node->value.lowerValue == "*") {
        state->ret() = left.multiply(right, &done);
        if (!done)
            throw NadaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }
    if (node->value.lowerValue == "/") {
        state->ret() = left.division(right, &done);
        if (!done)
            throw NadaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }

    if (node->value.lowerValue == "mod") {
        state->ret() = left.modulo(right, &done);
        if (!done)
            throw NadaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }

    if (node->value.lowerValue == "and") {
        bool result = left.logicalAnd(right, &done);
        if (!done)
            throw NadaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == "or") {
        bool result = left.logicalOr(right, &done);
        if (!done)
            throw NadaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(result);
        return state->ret();
    }

    if (node->value.lowerValue == "xor") {
        bool result = left.logicalXor(right, &done);
        if (!done)
            throw NadaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
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
    state->ret() = executeState(node->children[0],state).unaryOperator(node->value.lowerValue,&done);

    // FIXME: runtime error
    // if (!done)
    return state->ret();
}

//-------------------------------------------------------------------------------------------------
NadaValue &NadaInterpreter::executeFunctionCall(const NadaParser::ASTNodePtr &node, NadaState *state)
{
    NadaValues values;
    for (const auto &node : node->children) {
        if (node->type == NadaParser::ASTNodeType::MethodContext)
            continue;
        values.push_back(executeState(node, state));
    }

    Nda::Symbol symbol;
    std::string typeName;
    if (node->type == NadaParser::ASTNodeType::StaticMethodCall) {
        typeName = node->children[0]->value.lowerValue;
    } else if (node->type == NadaParser::ASTNodeType::InstanceMethodCall) {
        if (!state->find(node->children[0]->value.lowerValue,symbol))
            throw NadaException(Nada::Error::UnknownSymbol,node->line,node->column, node->children[0]->value.lowerValue);
        typeName = symbol.typeName.lowerValue;
    }

    if (!state->hasFunction(typeName, node->value.lowerValue,values)) {
        state->ret().reset();
        return state->ret(); // TODO: ERROR
    }

    auto &fnc = state->function(typeName, node->value.lowerValue,values);

    if (fnc.block) {
        state->pushStack(NadaSymbolTable::LocalScope);
        assert(values.size() == fnc.parameters.size());

        /*
                parameter: "x"  : "any"
                value:     "42" : Type = Natural

                Push to stack   : declare x : Natural := 42;
            */

        // Creating "this":
        if (node->type == NadaParser::ASTNodeType::InstanceMethodCall) {
            state->define("this", symbol.typeName.lowerValue);
            // TODO: if !define -> runtime error!
            NadaValue &valueRef = state->valueRef("this");
            valueRef.fromReference(symbol.value);
        }
        for (int i = 0; i< fnc.parameters.size(); i++) {
            state->define(fnc.parameters[i].name, fnc.parameters[i].type);
            // TODO: if !define -> runtime error!
            NadaValue &valueRef = state->valueRef(fnc.parameters[i].name);
            if (fnc.parameters[i].mode == Nda::OutMode) {
                valueRef.fromReference(&values[i]);
            } else {
                valueRef.assign(values[i]);
            }
        }

        executeState(fnc.block, state);

        if (mExecState == ReturnState)
            mExecState = RunState;
        state->popStack();
    } else
        fnc.nativeCallback(fnc.fncValues(values));

    return state->ret();
}

//-------------------------------------------------------------------------------------------------
void NadaInterpreter::defineProcedure(const NadaParser::ASTNodePtr &node, NadaState *state)
{
    assert(node->type == NadaParser::ASTNodeType::Procedure);
    assert(node->children.size() >= 2);

    std::string name = node->value.lowerValue;
    std::string typeName;
    int parameterIndex = 0;
    if (node->children[0]->type == NadaParser::ASTNodeType::MethodContext) {
        typeName = node->children[0]->value.lowerValue;
        parameterIndex = 1;
    }

    auto parameters = node->children[parameterIndex+0];
    auto block      = node->children[parameterIndex+1];

    assert(parameters->type == NadaParser::ASTNodeType::FormalParameters);
    assert(block->type      == NadaParser::ASTNodeType::Block);

    Nda::FncParameters fncParameters;
    for (const auto &p : parameters->children) {
        assert(p->children.size() >= 1); // TODO: in/out, child # 2
        Nda::ParameterMode mode = Nda::InMode;
        if (p->children.size() == 2) {
            assert(p->children[1]->type == NadaParser::ASTNodeType::FormalParameterMode);
            mode = p->children[1]->value.lowerValue == "out" ? Nda::OutMode : Nda::InMode;
        }
        fncParameters.push_back({p->value.lowerValue,p->children[0]->value.lowerValue,mode});
    }

    state->bind(typeName,name,fncParameters,block);
}

//-------------------------------------------------------------------------------------------------
void NadaInterpreter::defineFunction(const NadaParser::ASTNodePtr &node, NadaState *state)
{
    assert(node->type == NadaParser::ASTNodeType::Function);
    assert(node->children.size() >= 3);

    std::string name = node->value.lowerValue;
    std::string typeName;
    int parameterIndex = 0;
    if (node->children[0]->type == NadaParser::ASTNodeType::MethodContext) {
        typeName = node->children[0]->value.lowerValue;
        parameterIndex = 1;
    }

    auto parameters = node->children[parameterIndex+0];
    auto returntype = node->children[parameterIndex+1];
    auto block      = node->children[parameterIndex+2];

    assert(parameters->type == NadaParser::ASTNodeType::FormalParameters);
    assert(block->type      == NadaParser::ASTNodeType::Block);

    Nda::FncParameters fncParameters;
    for (const auto &p : parameters->children) {
        Nda::ParameterMode mode = Nda::InMode;
        assert(p->children.size() >= 1);
        if (p->children.size() == 2) {
            assert(p->children[1]->type == NadaParser::ASTNodeType::FormalParameterMode);
            mode = p->children[1]->value.lowerValue == "out" ? Nda::OutMode : Nda::InMode;
        }
        fncParameters.push_back({p->value.lowerValue,p->children[0]->value.lowerValue, mode});
    }

    state->bind(typeName,node->value.lowerValue,fncParameters,block);
}

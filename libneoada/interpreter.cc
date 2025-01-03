#include <assert.h>
#include <iostream>
#include "interpreter.h"
#include "exception.h"

//-------------------------------------------------------------------------------------------------
NdaInterpreter::NdaInterpreter(NdaState *state)
    : mState(state)
{
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaInterpreter::execute(const NdaParser::ASTNodePtr &node, NdaState *state)
{
    assert(node);
    if (!state && !mState)
        return NdaVariant();

    mExecState = RunState;
    executeState(node, state ? state : mState);

    return (state ? state : mState)->ret();
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaInterpreter::executeState(const NdaParser::ASTNodePtr &node, NdaState *state)
{
    assert(state);
    assert(node);

    switch (node->type) {
    case NdaParser::ASTNodeType::Program:
        for (const auto &child : node->children) {
            executeState(child, state);
            if (mExecState == ReturnState)
                break;
        }
        break;
    case NdaParser::ASTNodeType::WithAddon:
        state->requestAddon(node->value.lowerValue);
        break;
    case NdaParser::ASTNodeType::Procedure:
        defineProcedure(node,state);
        break;
    case NdaParser::ASTNodeType::Function:
        defineFunction(node,state);
        break;
    case NdaParser::ASTNodeType::Block:
        state->pushScope(node->parent->type == NdaParser::ASTNodeType::WhileLoop ? NadaSymbolTable::LoopScope : NadaSymbolTable::ConditionalScope);
        for (const auto &child : node->children) {
            executeState(child, state);
            if (mExecState == ReturnState)
                break;
            if (mExecState == BreakState)
                break;
            if (mExecState == ContinueState)

            break;
        }
        state->ret().dereference();
        state->popScope();
        break;
    case NdaParser::ASTNodeType::Expression: // just "()"
        assert(node->children.size() == 1);
        return executeState(node->children[0],state);
        break;
    case NdaParser::ASTNodeType::VolatileDeclaration:
    case NdaParser::ASTNodeType::Declaration:
        assert(node->children.size() >= 1);
        if (!state->define(node->value.lowerValue, node->children[0]->value.lowerValue, node->type == NdaParser::ASTNodeType::VolatileDeclaration)) {
            state->ret().reset();
            return state->ret(); // FIXME: Runtime-Error
        }
        if (node->children.size() == 2) {
            auto &value = state->valueRef(node->value.lowerValue);
            NdaVariant initialValue;

            state->ret().reset();
            executeState(node->children[1],state);

            if (!value.assign(state->ret()))
                throw NdaException(Nada::Error::AssignmentError,node->line,node->column, node->value.displayValue);
        }
        break;
    case NdaParser::ASTNodeType::IfStatement: {
        assert(node->children.size() >= 2);
        bool conditionValid;
        bool condition = executeState(node->children[0], state).toBool(&conditionValid);
        // if (!conditionValid) // TODO: runtime error
        if (condition) {
            return executeState(node->children[1],state);
        } else {
            int elsIfIndex = 2;
            while (!condition &&
                   node->children.size() > (size_t)elsIfIndex &&
                   node->children[elsIfIndex]->type == NdaParser::ASTNodeType::Elsif)
            {
                assert(node->children[elsIfIndex]->children.size() == 2);
                condition = executeState(node->children[elsIfIndex]->children[0], state).toBool(&conditionValid);
                if (condition)
                    return executeState(node->children[elsIfIndex]->children[1],state);
                elsIfIndex++;
            }

            if (!condition && node->children.back()->type == NdaParser::ASTNodeType::Else) {
                assert(node->children.back()->children.size() == 1);
                return executeState(node->children.back()->children[0],state);
            }

        }

    } break;
    case NdaParser::ASTNodeType::Else: {
        assert(node->children.size() == 1);
        return executeState(node->children[0],state);
    } break;
    case NdaParser::ASTNodeType::WhileLoop: {
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
            if (mExecState == ReturnState) {
                break;
            }
            if (mExecState == ContinueState) {
                mExecState = RunState;
            }
        } while (condition && conditionValid);
    } break;
    case NdaParser::ASTNodeType::ForLoop: {
        assert(node->children.size() == 2); // range + body
        if (node->children[0]->type == NdaParser::ASTNodeType::Range)
            executeForLoopRange(node,state);
        return state->ret();
    } break;
    case NdaParser::ASTNodeType::Return: {
        if (node->children.size() == 1)
            executeState(node->children[0], state);
        mExecState = ReturnState;
        return state->ret();
    } break;
    case NdaParser::ASTNodeType::Break: {
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
    case NdaParser::ASTNodeType::Continue: {
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
    case NdaParser::ASTNodeType::FunctionCall: {
        executeFunctionCall(node, state);
    }   break;
    case NdaParser::ASTNodeType::Assignment: {
        assert(node->children.size() == 2);

        executeState(node->children[0], state);

        if (state->ret().myType() != Nda::Reference)
            throw NdaException(Nada::Error::InvalidAssignment,node->line,node->column, node->value.displayValue);

        auto targetValue = state->ret();

        executeState(node->children[1], state);

        if (!targetValue.assign(state->ret()))
            throw NdaException(Nada::Error::AssignmentError,node->line,node->column, node->value.displayValue);
    }   break;
    case NdaParser::ASTNodeType::Literal:
        state->ret().fromString(state->stringType(),node->value.displayValue);
        break;
    case NdaParser::ASTNodeType::ListLiteral: {
        NdaVariant ret;
        ret.initType(state->listType());
        for (const auto &child : node->children) {
            ret.appendToList(executeState(child, state));
        }
        state->ret() = ret;
    } break;
    case NdaParser::ASTNodeType::BooleanLiteral:
        if (node->variantCache) {
            state->ret() = *node->variantCache;
        } else {
            state->ret().fromBool(state->booleanType(),node->value.lowerValue == "true");
            node->variantCache = new NdaVariant(state->ret());
        }
        break;
    case NdaParser::ASTNodeType::Number: {
        executeNumber(node,state);
    }   break;
    case NdaParser::ASTNodeType::BinaryOperator: {
        return evaluateBinaryOperator(node, state);
    }   break;
    case NdaParser::ASTNodeType::UnaryOperator: {
        return evaluateUnaryOperator(node, state);
    }   break;
    case NdaParser::ASTNodeType::Identifier: {
        auto *value = state->valuePtr(node->value.lowerValue);
        if (value)
            state->ret().fromReference(state->referenceType(),value);
        else
            state->ret().reset();
        // TODO: runtime error
    }   break;
    case NdaParser::ASTNodeType::StaticMethodCall: {
        executeFunctionCall(node, state);
    }   break;
    case NdaParser::ASTNodeType::InstanceMethodCall: {
        executeFunctionCall(node, state);
    }   break;
    case NdaParser::ASTNodeType::AccessOperator: {
        assert(node->children.size() == 2);
        assert(node->children[0]->type == NdaParser::ASTNodeType::Identifier);

        if (state->typeOf(node->children[0]->value.lowerValue) == Nda::Undefined)
            throw NdaException(Nada::Error::UnknownSymbol,node->line,node->column, node->value.displayValue);

        if (state->typeOf(node->children[0]->value.lowerValue) != Nda::List)
            throw NdaException(Nada::Error::InvalidContainerType,node->line,node->column, node->value.displayValue);

        auto &targetList = state->valueRef(node->children[0]->value.lowerValue);

        executeState(node->children[1], state);

        bool done = false;
        int64_t index = state->ret().toInt64(&done);
        if (!done)
           throw NdaException(Nada::Error::InvalidAccessValue,node->line,node->column, node->value.displayValue);

        if (index < 0)
            throw NdaException(Nada::Error::InvalidAccessValue,node->line,node->column, node->value.displayValue);

        if (index >= targetList.listSize())
            throw NdaException(Nada::Error::InvalidAccessValue,node->line,node->column, node->value.displayValue);

        auto &targetValue = targetList.writeAccess((int)index);
        state->ret().fromReference(state->referenceType(), &targetValue);
    }   break;
    default:
        assert(0 && "not yet implemented");
        break;
    }

    return state->ret();
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaInterpreter::executeNumber(const NdaParser::ASTNodePtr &node, NdaState *state)
{
    if (node->variantCache) {
        state->ret() = *node->variantCache;
        return state->ret();
    }

    auto &ret = state->ret();
    auto identType = NdaVariant::numericType(node->value.lowerValue); // _B -> _b

    bool done = false;
    switch(identType) {
    case Nda::Number:
        done = ret.fromNumberLiteral(state->typeByName("number")   ,node->value.lowerValue);
        break;
    case Nda::Natural:
        done = ret.fromNaturalLiteral(state->typeByName("natural") ,node->value.lowerValue);
        break;
    case Nda::Supernatural:
        done = ret.fromNaturalLiteral(state->typeByName("supernatural") ,node->value.lowerValue);
        break;
    case Nda::Byte:
        done = ret.fromNaturalLiteral(state->typeByName("byte") ,node->value.lowerValue);
        break;
    default: break;
    }

    if (!done)
        throw NdaException(Nada::Error::InvalidNumericValue,node->line,node->column, node->value.displayValue);

    node->variantCache = new NdaVariant(ret);
    return ret;
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaInterpreter::executeForLoopRange(const NdaParser::ASTNodePtr &node, NdaState *state)
{
    assert(node->children.size() == 2);

    std::string varName = node->value.displayValue;
    assert(varName.length() > 0);

    assert(node->children[0]->children.size() == 2); // from .. to

    int64_t from,to;

    // TODO: runtime error handling
    NdaVariant rangeStart = executeState(node->children[0]->children[0], state);
    NdaVariant rangeEnd   = executeState(node->children[0]->children[1], state);

    from = rangeStart.toInt64();
    to   = rangeEnd.toInt64();

    state->pushScope(NadaSymbolTable::LoopScope);
    state->define(varName,"Natural");
    auto &valueRef = state->valueRef(varName);
    for (int64_t i = from; i<=to; i++) {          
        valueRef.fromNatural(state->typeByName("natural"),i);
        executeState(node->children[1],state);

        if (mExecState == BreakState) {
            mExecState = RunState;
            break;
        }
        if (mExecState == ContinueState) {
            mExecState = RunState;
        }
    }

    state->ret().dereference();
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
NdaVariant &NdaInterpreter::evaluateBinaryOperator(const NdaParser::ASTNodePtr &node, NdaState *state)
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
            throw NdaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(state->booleanType(),result);
        return state->ret();
    }

    if (node->value.lowerValue == "<") {
        bool result = left.lessThen(right, &done);
        if (!done)
            throw NdaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(state->booleanType(),result);
        return state->ret();
    }

    if (node->value.lowerValue == ">=") {
        bool result1 = left.greaterThen(right, &done);
        bool result2 = result1 || left.equal(right);

        if (result1 || result2) {
            state->ret().fromBool(state->booleanType(),result1);
            return state->ret();
        }
        if (!done)
            throw NdaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);

        state->ret().fromBool(state->booleanType(),false);
        return state->ret();
    }

    if (node->value.lowerValue == "<=") {
        bool result1 = left.lessThen(right, &done);
        if (!done)
            throw NdaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);

        bool result2 = result1 || left.equal(right, &done);
        if (!done)
            throw NdaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);

        if (result1 || result2) {
            state->ret().fromBool(state->booleanType(),result1 || result2);
            return state->ret();
        }


        state->ret().fromBool(state->booleanType(),false);
        return state->ret();
    }

    if (node->value.lowerValue == "=") {
        bool result = left.equal(right, &done);
        if (!done)
            throw NdaException(Nada::Error::IllegalComparison,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(state->booleanType(),result);
        return state->ret();
    }

    if (node->value.lowerValue == "<>") {
        bool result = !left.equal(right, &done);
        if (!done)
            throw NdaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(state->booleanType(),result);
        return state->ret();
    }

    if (node->value.lowerValue == "&") {
        state->ret() = left.concat(right, &done);
        if (!done)
            throw NdaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }

    if (node->value.lowerValue == "-") {
        state->ret() = left.subtract(right, &done);
        if (!done)
            throw NdaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }

    if (node->value.lowerValue == "+") {
        state->ret() = left.add(right, &done);
        if (!done)
            throw NdaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }

    if (node->value.lowerValue == "*") {
        state->ret() = left.multiply(right, &done);
        if (!done)
            throw NdaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }
    if (node->value.lowerValue == "/") {
        state->ret() = left.division(right, &done);
        if (!done)
            throw NdaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }

    if (node->value.lowerValue == "mod") {
        state->ret() = left.modulo(right, &done);
        if (!done)
            throw NdaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        return state->ret();
    }

    if (node->value.lowerValue == "and") {
        bool result = left.logicalAnd(right, &done);
        if (!done)
            throw NdaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(state->booleanType(),result);
        return state->ret();
    }

    if (node->value.lowerValue == "or") {
        bool result = left.logicalOr(right, &done);
        if (!done)
            throw NdaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(state->booleanType(),result);
        return state->ret();
    }

    if (node->value.lowerValue == "xor") {
        bool result = left.logicalXor(right, &done);
        if (!done)
            throw NdaException(Nada::Error::OperatorTypeError,node->line,node->column, node->value.displayValue);
        state->ret().fromBool(state->booleanType(),result);
        return state->ret();
    }

    assert(0 && "not yet implemented");
    return state->ret();
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaInterpreter::evaluateUnaryOperator(const NdaParser::ASTNodePtr &node, NdaState *state)
{
    assert(node->children.size() == 1);

    if (node->value.lowerValue == "#") {
        state->ret().fromNatural(state->naturalType(), executeState(node->children[0],state).lengthOperator());
    } else {
        bool done;
        state->ret() = executeState(node->children[0],state).unaryOperator(node->value.lowerValue,&done);
    }

    // FIXME: runtime error
    // if (!done)
    return state->ret();
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaInterpreter::executeFunctionCall(const NdaParser::ASTNodePtr &node, NdaState *state)
{
    NadaValues values;
    for (const auto &node : node->children) {
        if (node->type == NdaParser::ASTNodeType::MethodContext)
            continue;
        values.push_back(executeState(node, state));
    }

    Nda::Symbol *symbol;
    std::string typeName;
    if (node->type == NdaParser::ASTNodeType::StaticMethodCall) {
        typeName = node->children[0]->value.lowerValue;
    } else if (node->type == NdaParser::ASTNodeType::InstanceMethodCall) {
        if (!state->find(node->children[0]->value.lowerValue,&symbol))
            throw NdaException(Nada::Error::UnknownSymbol,node->line,node->column, node->children[0]->value.lowerValue);
        typeName = symbol->type->name.lowerValue;
    }

    if (!state->hasFunction(typeName, node->value.lowerValue,values)) {
        state->ret().reset();
        throw NdaException(Nada::Error::UnknownSymbol,node->line,node->column, typeName + ":" + node->value.lowerValue);
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
        if (node->type == NdaParser::ASTNodeType::InstanceMethodCall) {
            state->define("this", symbol->type);
            // TODO: if !define -> runtime error!
            NdaVariant &valueRef = state->valueRef("this");
            valueRef.fromReference(state->typeByName("reference"),symbol->value);
        }
        for (int i = 0; i< (int)fnc.parameters.size(); i++) {
            state->define(fnc.parameters[i].name, fnc.parameters[i].type);
            // TODO: if !define -> runtime error!
            NdaVariant &valueRef = state->valueRef(fnc.parameters[i].name);
            if (fnc.parameters[i].mode == Nda::OutMode) {
                valueRef.fromReference(state->typeByName("reference"),&values[i]);
            } else {
                valueRef.assign(values[i]);
            }
        }

        executeState(fnc.block, state);

        if (mExecState == ReturnState)
            mExecState = RunState;

        state->ret().dereference();
        state->popStack();
    } else {
        auto parameters = fnc.fncValues(values);

        // Creating "this":
        if (node->type == NdaParser::ASTNodeType::InstanceMethodCall) {
            NdaVariant thisRef;
            thisRef.fromReference(state->typeByName("reference"),symbol->value);
            parameters["this"] = thisRef;
        }

        if (fnc.nativeFncCallback)
            fnc.nativeFncCallback(parameters, state->ret());
        else
            fnc.nativePrcCallback(parameters);
    }

    return state->ret();
}

//-------------------------------------------------------------------------------------------------
void NdaInterpreter::defineProcedure(const NdaParser::ASTNodePtr &node, NdaState *state)
{
    assert(node->type == NdaParser::ASTNodeType::Procedure);
    assert(node->children.size() >= 2);

    std::string name = node->value.lowerValue;
    std::string typeName;
    int parameterIndex = 0;
    if (node->children[0]->type == NdaParser::ASTNodeType::MethodContext) {
        typeName = node->children[0]->value.lowerValue;
        parameterIndex = 1;
    }

    auto parameters = node->children[parameterIndex+0];
    auto block      = node->children[parameterIndex+1];

    assert(parameters->type == NdaParser::ASTNodeType::FormalParameters);
    assert(block->type      == NdaParser::ASTNodeType::Block);

    Nda::FncParameters fncParameters;
    for (const auto &p : parameters->children) {
        assert(p->children.size() >= 1); // TODO: in/out, child # 2
        Nda::ParameterMode mode = Nda::InMode;
        if (p->children.size() == 2) {
            assert(p->children[1]->type == NdaParser::ASTNodeType::FormalParameterMode);
            mode = p->children[1]->value.lowerValue == "out" ? Nda::OutMode : Nda::InMode;
        }
        fncParameters.push_back({p->value.lowerValue,p->children[0]->value.lowerValue,mode});
    }

    state->bind(typeName,name,fncParameters,block);
}

//-------------------------------------------------------------------------------------------------
void NdaInterpreter::defineFunction(const NdaParser::ASTNodePtr &node, NdaState *state)
{
    assert(node->type == NdaParser::ASTNodeType::Function);
    assert(node->children.size() >= 3);

    std::string name = node->value.lowerValue;
    std::string typeName;
    int parameterIndex = 0;
    if (node->children[0]->type == NdaParser::ASTNodeType::MethodContext) {
        typeName = node->children[0]->value.lowerValue;
        parameterIndex = 1;
    }

    auto parameters = node->children[parameterIndex+0];
    auto returntype = node->children[parameterIndex+1];
    auto block      = node->children[parameterIndex+2];

    assert(parameters->type == NdaParser::ASTNodeType::FormalParameters);
    assert(block->type      == NdaParser::ASTNodeType::Block);

    Nda::FncParameters fncParameters;
    for (const auto &p : parameters->children) {
        Nda::ParameterMode mode = Nda::InMode;
        assert(p->children.size() >= 1);
        if (p->children.size() == 2) {
            assert(p->children[1]->type == NdaParser::ASTNodeType::FormalParameterMode);
            mode = p->children[1]->value.lowerValue == "out" ? Nda::OutMode : Nda::InMode;
        }
        fncParameters.push_back({p->value.lowerValue,p->children[0]->value.lowerValue, mode});
    }

    state->bind(typeName,node->value.lowerValue,fncParameters,block);
}

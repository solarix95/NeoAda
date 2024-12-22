
#include "parser.h"
#include "exception.h"

//-------------------------------------------------------------------------------------------------
NadaParser::NadaParser(NadaLexer &lexer)
    : mLexer(lexer)
    , mState(ParserState::None)
{
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parse(const std::string &script) {

    mLexer.setScript(script);

    auto programNode = std::make_shared<ASTNode>(ASTNodeType::Program);
    while (mLexer.nextToken()) {
        auto declarationNode = parseStatement();
        if (declarationNode)
            ASTNode::addChild(programNode,declarationNode);
    }

    return programNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseStatement()
{
    if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "declare") {
        return parseSeparator(parseDeclaration());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "while") {
        return parseSeparator(parseWhileLoop());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "for") {
        return parseSeparator(parseForLoop());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "if") {
        return parseSeparator(parseIfStatement());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "return") {
        return parseSeparator(parseReturn());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "break") {
        return parseSeparator(parseBreak());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "continue") {
        return parseSeparator(parseContinue());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "procedure") {
        return parseSeparator(parseProcedureOrFunction());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "function") {
        return parseSeparator(parseProcedureOrFunction());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Identifier) {
        return parseSeparator(parseIdentifier());
    }

    if (mLexer.atEnd())
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());
    throw NadaException(Nada::Error::InvalidStatement,mLexer.line(), mLexer.column(),mLexer.token());
    return NadaParser::ASTNodePtr();
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseDeclaration()
{
    auto declarationNode = std::make_shared<ASTNode>(ASTNodeType::Declaration);

    mLexer.nextToken(); // skip "declare"

    if (mLexer.tokenType() != NadaLexer::TokenType::Identifier)
        throw NadaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

    declarationNode->value = mLexer.token(); // set variable name

    mLexer.nextToken(); // skip identifier

    if (mLexer.token() != ":")
        throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

    mLexer.nextToken(); // skip ':'

    if (mLexer.tokenType() != NadaLexer::TokenType::Identifier)
        throw NadaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

    auto typeNode = std::make_shared<ASTNode>(ASTNodeType::Identifier, mLexer.token());
    ASTNode::addChild(declarationNode,typeNode);

    // Erwarte ";" oder ":="
    if (mLexer.token(1) == ":=") {
        mLexer.nextToken();       // springe zu   ":="
        if (!mLexer.nextToken())  // springe nach ":="
            throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

        auto expressionNode = parseExpression();
        if (!expressionNode)
            return NadaParser::ASTNodePtr();
        ASTNode::addChild(declarationNode,expressionNode);
    }

    return declarationNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseIdentifier()
{
    auto identifierNode = std::make_shared<ASTNode>(ASTNodeType::Identifier,mLexer.token());

    mLexer.nextToken();

    if (mLexer.token() == ":=") {
        identifierNode->type = ASTNodeType::Assignment;

        if (!mLexer.nextToken())
            throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

        auto expressionNode = parseExpression();
        if (!expressionNode)
            return NadaParser::ASTNodePtr();
        ASTNode::addChild(identifierNode,expressionNode);

    } else if (mLexer.token() == "(") {
        identifierNode->type = ASTNodeType::FunctionCall;
        parseFunctionCall(identifierNode);
    } else {
        throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
    }
    return identifierNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseProcedureOrFunction()

//                         for procedures AND functions

{
    bool isFunction = mLexer.token() == "function";

    auto procedureNode = std::make_shared<ASTNode>(isFunction ? ASTNodeType::Function :
                                                                ASTNodeType::Procedure,
                                                    mLexer.token());

    if (!mLexer.nextToken())
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    if (mLexer.tokenType() != NadaLexer::TokenType::Identifier)
        throw NadaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

    procedureNode->value = mLexer.token();

    if (!mLexer.nextToken())
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    auto parameterNode = parseFormalParameterList();
    ASTNode::addChild(procedureNode,parameterNode);

    if (isFunction) {
        // function Add(a : in Natural; b : in Natural) return Natural is
        //                                                |       |
        if (mLexer.token() != "return")
            throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
        mLexer.nextToken();

        if (mLexer.tokenType() != NadaLexer::TokenType::Identifier)
            throw NadaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

        auto returnNode = std::make_shared<ASTNode>(ASTNodeType::ReturnType, mLexer.token());
        ASTNode::addChild(procedureNode,returnNode);

        mLexer.nextToken();
    }

    if (mLexer.token() != "is")
        throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

    if (!mLexer.nextToken())
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());


    if (mLexer.token() != "begin")
        throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
    mLexer.nextToken();

    auto blockNode = std::make_shared<ASTNode>(ASTNodeType::Block);
    while (mLexer.token() != "end") {
        auto nextStatement = parseStatement();
        if (nextStatement)
            ASTNode::addChild(blockNode,nextStatement);
        mLexer.nextToken();
    }

    ASTNode::addChild(procedureNode,blockNode);

    return procedureNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseWhileLoop()
{
    auto whileNode = std::make_shared<ASTNode>(ASTNodeType::WhileLoop);
    if (!mLexer.nextToken())
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    auto condition = parseExpression();

    if (!condition) // hmm assert?
        throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column(),mLexer.token());

    ASTNode::addChild(whileNode,condition);

    mLexer.nextToken();
    if (mLexer.token() != "loop")
        throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
    mLexer.nextToken();

    // 3. Parse den Block (Statements zwischen 'loop' und 'end loop')
    auto blockNode = std::make_shared<ASTNode>(ASTNodeType::Block);
    ASTNode::addChild(whileNode,blockNode);

    while (mLexer.token() != "end") {
        auto nextStatement = parseStatement();
        if (nextStatement)
            ASTNode::addChild(blockNode,nextStatement);
        mLexer.nextToken();
    }
    mLexer.nextToken(); // Überspringe end 'loop'

    return whileNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseForLoop()
{
    auto forNode = std::make_shared<ASTNode>(ASTNodeType::ForLoop);

    // consume "for"
    if (!mLexer.nextToken())
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    if (mLexer.tokenType() != NadaLexer::TokenType::Identifier)
        throw NadaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

    std::string loopVar = mLexer.token();

    if (!mLexer.nextToken())
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    if (mLexer.token() != "in")
        throw NadaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"in");

    if (!mLexer.nextToken())
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    auto iterableOrRangeNode = parseIterableOrRange();
    if (!iterableOrRangeNode)
        throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    mLexer.nextToken();
    if (mLexer.token() != "loop")
        throw NadaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"loop");

    mLexer.nextToken(); // Consume "loop"

    auto bodyNode = parseBlockEnd("end");
    if (!bodyNode)
        throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    if (mLexer.token() != "end")
        throw NadaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"end");
    mLexer.nextToken(); // Consume "end"

    if (mLexer.token() != "loop")
        throw NadaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"loop");

    // "loop" consumed by "parseSeparator()"

    forNode->value = loopVar;
    ASTNode::addChild(forNode,iterableOrRangeNode);
    ASTNode::addChild(forNode,bodyNode);

    return forNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseIfStatement()
{
    // Erwarte das Schlüsselwort "if"
    if (mLexer.token() != "if")
        throw NadaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"if");

    mLexer.nextToken(); // Consume "if"

    // 1. Parse die Bedingung
    auto conditionNode = parseExpression();
    if (!conditionNode) {
        throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());
    }

    // Erwarte "then"
    mLexer.nextToken();
    if (mLexer.token() != "then")
        throw NadaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"then");

    mLexer.nextToken(); // Consume "then"

    // Erstelle den IfStatement-Knoten
    auto ifNode = std::make_shared<ASTNode>(ASTNodeType::IfStatement);
    ASTNode::addChild(ifNode,conditionNode);

    // 2. Parse den "then"-Block
    auto thenBlock = parseBlockEnd("elsif", "else");
    if (!thenBlock)
        throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    ASTNode::addChild(ifNode,thenBlock);

    // 3. Optional: Parse "elsif"-Blöcke
    while (mLexer.token() == "elsif") {
        mLexer.nextToken(); // Consume "elsif"
        auto elsifCondition = parseExpression();
        if (!elsifCondition) // assert?
            throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

        mLexer.nextToken();
        if (mLexer.token() != "then")
            throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

        mLexer.nextToken(); // Consume "then"

        auto elsifBlock = parseBlockEnd("elsif", "else");
        if (!elsifBlock)
            throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

        auto elsifNode = std::make_shared<ASTNode>(ASTNodeType::Elsif);
        ASTNode::addChild(elsifNode,elsifCondition);
        ASTNode::addChild(elsifNode,elsifBlock);
        ASTNode::addChild(ifNode,elsifNode);
    }

    // 4. Optional: Parse "else"-Block
    if (mLexer.token() == "else") {
        mLexer.nextToken(); // Consume "else"

        auto elseBlock = parseBlockEnd("end");
        if (!elseBlock)
            throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

        auto elseNode = std::make_shared<ASTNode>(ASTNodeType::Else);
        ASTNode::addChild(elseNode,elseBlock);
        ASTNode::addChild(ifNode,elseNode);
    }

    // 5. Erwarte "end if"
    if (mLexer.token() != "end")
        throw NadaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(), "end");

    if (!mLexer.nextToken()) // Consume "end"
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    if (mLexer.token() != "if")
        throw NadaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(), "if");
    return ifNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseReturn()
{
    auto returnNode = std::make_shared<ASTNode>(ASTNodeType::Return);
    if (!mLexer.nextToken())
        throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

    auto expression = parseExpression();

    if (!expression) // assert?
        throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    ASTNode::addChild(returnNode,expression);
    return returnNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseBreak()
//             break [when expression]
{
    auto breakNode = std::make_shared<ASTNode>(ASTNodeType::Break);
    if (mLexer.token(1) != "when")
        return breakNode;

    mLexer.nextToken();        // step to   "when"
    if (!mLexer.nextToken())   // step over "when
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    auto expression = parseExpression();

    if (!expression) // assert?
        throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    ASTNode::addChild(breakNode,expression);
    return breakNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseContinue()
{
    auto contNode = std::make_shared<ASTNode>(ASTNodeType::Continue);

    if (mLexer.token(1) != "when")
        return contNode;

    mLexer.nextToken();        // step to   "when"
    if (!mLexer.nextToken())   // step over "when
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    auto expression = parseExpression();

    if (!expression) // assert?
        throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    ASTNode::addChild(contNode,expression);

    return contNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseBlockEnd(const std::string &endToken1, const std::string &endToken2)
{
    auto blockNode = std::make_shared<ASTNode>(ASTNodeType::Block);

    while (mLexer.token() != "end" && mLexer.token() != endToken1 && (endToken2.empty() || mLexer.token() != endToken2)) {
        auto statementNode = parseStatement();
        if (!statementNode) {
            throw NadaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column(),mLexer.token());
        }
        if (mLexer.token() != ";")
            throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

        ASTNode::addChild(blockNode,statementNode);
        if (!mLexer.nextToken())
            throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());
    }
    return blockNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseSeparator(const std::shared_ptr<ASTNode> &currentNode)
{
    if (!currentNode)
        return currentNode;

    if (!mLexer.nextToken())
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    if (mLexer.token() != ";")
        throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

    return currentNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseFormalParameterList()
{
    auto parametersNode = std::make_shared<ASTNode>(ASTNodeType::FormalParameters);

    if (mLexer.token() != "(")
        return parametersNode;

    if (!mLexer.nextToken())
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    while (mLexer.token() != ")") {

        if (mLexer.tokenType() != NadaLexer::TokenType::Identifier)
            throw NadaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

        std::string paramName = mLexer.token();
        mLexer.nextToken(); // Consume name

        if (mLexer.token() != ":")
            throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
        if (!mLexer.nextToken()) // Consume ":"
            throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

        std::string paramMode; // "in" / "out"
        if (mLexer.tokenType() == NadaLexer::TokenType::Keyword) {
            paramMode = mLexer.token();
            mLexer.nextToken();
        }

        // Parameter Type e.g. "Natural",..
        if (mLexer.tokenType() != NadaLexer::TokenType::Identifier)
            throw NadaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

        std::string paramType = mLexer.token();

        auto parameterNode = std::make_shared<ASTNode>(ASTNodeType::FormalParameter, paramName);
        auto parameterType = std::make_shared<ASTNode>(ASTNodeType::Identifier,      paramType);
        ASTNode::addChild(parameterNode,parameterType);

        if (!paramMode.empty()) {
            auto parameterMode = std::make_shared<ASTNode>(ASTNodeType::FormalParameterMode, paramMode);
            ASTNode::addChild(parameterNode,parameterMode);
        }

        mLexer.nextToken();     // Consume parameterType

        ASTNode::addChild(parametersNode,parameterNode);

        if (mLexer.token() == ";")
            mLexer.nextToken(); // Consume ","
    }

     mLexer.nextToken(); // Consume ")"
    return parametersNode;
}

//-------------------------------------------------------------------------------------------------
std::string NadaParser::nodeTypeToString(ASTNodeType type)
{
    switch (type) {
    case ASTNodeType::Program:      return "Program";
    case ASTNodeType::Procedure:    return "Procedure";
    case ASTNodeType::Function:     return "Function";
    case ASTNodeType::FormalParameters:  return "Parameters";
    case ASTNodeType::FormalParameter :  return "Parameter";
    case ASTNodeType::Declaration:  return "Declaration";
    case ASTNodeType::Assignment:   return "Assignment";
    case ASTNodeType::Expression:   return "Expression";
    case ASTNodeType::ExpressionList: return "ExpressionList";
    case ASTNodeType::Literal:      return "Literal";
    case ASTNodeType::Number:       return  "Number";
    case ASTNodeType::Identifier:   return "Identifier";
    case ASTNodeType::UnaryOperator:  return "UnaryOperator";
    case ASTNodeType::BinaryOperator: return "BinaryOperator";
    case ASTNodeType::FunctionCall: return "FunctionCall";
    case ASTNodeType::IfStatement:  return "If";
    case ASTNodeType::Else:         return "Else";
    case ASTNodeType::Elsif:        return "ElseIf";
    case ASTNodeType::WhileLoop:    return "WhileLoop";
    case ASTNodeType::Loop:         return "Loop";
    case ASTNodeType::ForLoop:      return "ForLoop";
    case ASTNodeType::Range:        return "Range";
    case ASTNodeType::Block:        return "Block";
    case ASTNodeType::Break:        return "Break";
    case ASTNodeType::Continue:     return "Continue";
    case ASTNodeType::Return:       return "Return";
    case ASTNodeType::ReturnType:   return "ReturnType";
    default: return "Unknown";
    }
}

//-------------------------------------------------------------------------------------------------
//                                             ASTNode
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
std::string NadaParser::ASTNode::serialize(int depth) const
{
    std::string indent(depth * 2, ' '); // Einrückung
    std::string result = indent + "Node(" + nodeTypeToString(type) + ", \"" + value.displayValue + "\")\n";
    for (const auto& child : children) {
        result += child->serialize(depth + 1);
    }
    return result;
}

//-------------------------------------------------------------------------------------------------
void NadaParser::ASTNode::addChild(std::shared_ptr<ASTNode> parent, std::shared_ptr<ASTNode> child)
{
    parent->children.push_back(child);
    child->parent = parent;

}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseExpression()
{
    auto left = parseSimpleExpression();

    while (mLexer.token(1) == "and" || mLexer.token(1) == "or" || mLexer.token(1) == "xor") {
        mLexer.nextToken();
        auto operatorNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperator, mLexer.token());
        ASTNode::addChild(operatorNode,left);
        mLexer.nextToken(); // Hole den Operator
        auto right = parseSimpleExpression();
        ASTNode::addChild(operatorNode,right);
        left = operatorNode;
    }

    return left;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseSimpleExpression()
{
    std::string token;
    NadaLexer::TokenType tokenType;

    if (!mLexer.token(token,tokenType))
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    bool hasUnaryOperator = (token == "+" || token == "-");
    std::string unaryOperator = token;

    if (hasUnaryOperator)
        mLexer.nextToken();

    auto left = parseTerm();

    if (hasUnaryOperator) {
        auto term = left;
        left = std::make_shared<ASTNode>(ASTNodeType::UnaryOperator,unaryOperator);
        ASTNode::addChild(left,term);
    }

    while (mLexer.token(1) == "="  || mLexer.token(1) == "/=" ||
           mLexer.token(1) == "+"  || mLexer.token(1) == "-"  || mLexer.token(1) == "&" ||
           mLexer.token(1) == "<"  || mLexer.token(1) == ">"  ||
           mLexer.token(1) == "<=" || mLexer.token(1) == ">=")
    {
        mLexer.nextToken();
        auto operatorNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperator, mLexer.token());
        ASTNode::addChild(operatorNode,left);
        mLexer.nextToken(); // Hole den Operator
        auto right = parseTerm();
        ASTNode::addChild(operatorNode,right);
        left = operatorNode;
    }

    return left;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseTerm()
{
    auto left = parseFactor();

    while (mLexer.token(1) == "*" || mLexer.token(1) == "/" || mLexer.token(1) == "mod" || mLexer.token(1) == "rem") {
        mLexer.nextToken();
        auto operatorNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperator, mLexer.token());
        ASTNode::addChild(operatorNode,left);
        mLexer.nextToken(); // Hole den Operator
        auto right = parseFactor();
        ASTNode::addChild(operatorNode,right);
        left = operatorNode;
    }

    return left;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseFactor()
{
    auto left = parsePrimary();
    if (mLexer.token(1) == "**") {
        mLexer.nextToken();
        auto operatorNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperator, mLexer.token());
        ASTNode::addChild(operatorNode,left);
        mLexer.nextToken(); // Hole den Potenzierungsoperator
        auto right = parsePrimary();
        ASTNode::addChild(operatorNode,right);
        return operatorNode;
    }

    return left;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parsePrimary()
{
    std::string token;
    NadaLexer::TokenType tokenType;

    if (!mLexer.token(token,tokenType))
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    bool hasUnaryOperator = (token == "+" || token == "-");
    std::string unaryOperator = token;

    if (hasUnaryOperator)
        mLexer.nextToken();

    if (!mLexer.token(token,tokenType))
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    auto node = NadaParser::ASTNodePtr();

    if (tokenType == NadaLexer::TokenType::Number) {
        node = std::make_shared<ASTNode>(ASTNodeType::Number, token);
    } else if (tokenType == NadaLexer::TokenType::BooleanLiteral) {
        node = std::make_shared<ASTNode>(ASTNodeType::BooleanLiteral, token);
    }else if (tokenType == NadaLexer::TokenType::String) {
        node = std::make_shared<ASTNode>(ASTNodeType::Literal, token);
    } else if (tokenType == NadaLexer::TokenType::Identifier) {
        node = std::make_shared<ASTNode>(ASTNodeType::Identifier, token);

        if (mLexer.token(1) == "(") {
            mLexer.nextToken();
            node = parseFunctionCall(node);
        }

    } else if (token == "(") {
        if (!mLexer.nextToken()) // Überspringe '('
            throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

        node = std::make_shared<ASTNode>(ASTNodeType::Expression);
        auto expression = parseExpression();
        ASTNode::addChild(node,expression);

        if (!mLexer.nextToken())
            throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

        if (mLexer.token() != ")") {
            throw NadaException(Nada::Error::UnexpectedClosure,mLexer.line(), mLexer.column(),mLexer.token());
        }
    } else
        throw NadaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),token);

    if (hasUnaryOperator) {
        // hmm: runtime error? Bool or Strings can't have unary operators!
        auto term = node;
        node = std::make_shared<ASTNode>(ASTNodeType::UnaryOperator,unaryOperator);
        ASTNode::addChild(node,term);
    }

    return node;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseFunctionCall(std::shared_ptr<ASTNode> &funcNode)
{
    funcNode->type = ASTNodeType::FunctionCall;

    if (!mLexer.nextToken()) // Überspringe '('
        throw NadaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    if (mLexer.token() != ")") {
        while (true) {
            auto argument = parseExpression();
            ASTNode::addChild(funcNode,argument);
            mLexer.nextToken() ;
            if (mLexer.token() == ",") {
                mLexer.nextToken(); // jump over ","
            } else {
                break;
            }
        }

        if (mLexer.token() != ")") {
            throw NadaException(Nada::Error::UnexpectedClosure,mLexer.line(), mLexer.column(),mLexer.token());
        }
    }

    return funcNode;
}

//-------------------------------------------------------------------------------------------------
NadaParser::ASTNodePtr NadaParser::parseIterableOrRange()
{
    if (mLexer.tokenType() == NadaLexer::TokenType::Number || mLexer.tokenType()  == NadaLexer::TokenType::Identifier) {
        auto start = parseExpression();

        if (mLexer.token(1) == "..") {
            mLexer.nextToken(); // step to   ".."
            mLexer.nextToken(); // step over ".."
            auto end = parseExpression();
            auto rangeNode = std::make_shared<ASTNode>(ASTNodeType::Range);
            rangeNode->children.push_back(start);
            rangeNode->children.push_back(end);
            return rangeNode;
        }
        return start; // Es ist eine Iterable (z.B. anArray)
    }

    throw NadaException(Nada::Error::InvalidRangeOrIterable,mLexer.line(), mLexer.column(),mLexer.token());
    return NadaParser::ASTNodePtr();
}


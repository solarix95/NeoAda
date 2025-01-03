
#include "parser.h"
#include "exception.h"
#include "variant.h"
#include <cassert>

//-------------------------------------------------------------------------------------------------
NdaParser::NdaParser(NdaLexer &lexer)
    : mLexer(lexer)
    , mState(ParserState::None)
{
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parse(const std::string &script) {

    mLexer.setScript(script);

    auto programNode = std::make_shared<ASTNode>(ASTNodeType::Program, mLexer.line(), mLexer.column());
    while (mLexer.nextToken()) {
        auto declarationNode = parseStatement();
        if (declarationNode)
            ASTNode::addChild(programNode,declarationNode);
    }

    return programNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseStatement()
{
    if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "declare") {
        return parseSeparator(parseDeclaration());
    } else if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "volatile") {
        return parseSeparator(parseDeclaration());
    }else if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "while") {
        return parseSeparator(parseWhileLoop());
    } else if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "for") {
        return parseSeparator(parseForLoop());
    } else if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "if") {
        return parseSeparator(parseIfStatement());
    } else if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "return") {
        return parseSeparator(parseReturn());
    } else if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "break") {
        return parseSeparator(parseBreak());
    } else if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "continue") {
        return parseSeparator(parseContinue());
    } else if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "procedure") {
        return parseSeparator(parseProcedureOrFunction());
    } else if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "function") {
        return parseSeparator(parseProcedureOrFunction());
    } else if (mLexer.tokenType() == NdaLexer::TokenType::Keyword && mLexer.token() == "with") {
        return parseSeparator(parseWith());
    } else if (mLexer.tokenType() == NdaLexer::TokenType::Identifier) {
        return parseSeparator(parseIdentifier());
    }

    if (mLexer.atEnd())
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());
    throw NdaException(Nada::Error::InvalidStatement,mLexer.line(), mLexer.column(),mLexer.token());
    return NdaParser::ASTNodePtr();
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseDeclaration()
{
    bool isVolatile = mLexer.token() == "volatile";
    auto declarationNode = std::make_shared<ASTNode>(isVolatile ?
                                                        ASTNodeType::VolatileDeclaration : ASTNodeType::Declaration,
                                                    mLexer.line(), mLexer.column());

    mLexer.nextToken(); // skip "declare"

    if (mLexer.tokenType() != NdaLexer::TokenType::Identifier)
        throw NdaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

    declarationNode->value = mLexer.token(); // set variable name

    mLexer.nextToken(); // skip identifier

    if (mLexer.token() != ":")
        throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

    mLexer.nextToken(); // skip ':'

    if (mLexer.tokenType() != NdaLexer::TokenType::Identifier)
        throw NdaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

    auto typeNode = std::make_shared<ASTNode>(ASTNodeType::Identifier, mLexer.line(), mLexer.column(), mLexer.token());
    ASTNode::addChild(declarationNode,typeNode);

    // Erwarte ";" oder ":="
    if (mLexer.token(1) == ":=") {
        mLexer.nextToken();       // springe zu   ":="
        if (!mLexer.nextToken())  // springe nach ":="
            throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

        auto expressionNode = parseExpression();
        if (!expressionNode)
            return NdaParser::ASTNodePtr();
        ASTNode::addChild(declarationNode,expressionNode);
    }

    return declarationNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseWith()
{
    std::string addonName;

    NdaLexer::TokenType nextNode = NdaLexer::TokenType::Identifier;

    while (mLexer.token(1) != ";") {
        mLexer.nextToken();
        if (mLexer.tokenType() != nextNode)
            throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
        if (nextNode == NdaLexer::TokenType::Identifier) {
            addonName = addonName + mLexer.token();
            nextNode = NdaLexer::TokenType::Separator;
        } else if (mLexer.token() == ".") {
            addonName = addonName + ".";
            nextNode = NdaLexer::TokenType::Identifier;
        } else
            throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
    }

    auto withNode = std::make_shared<ASTNode>(ASTNodeType::WithAddon,mLexer.line(), mLexer.column(), addonName);
    return withNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseIdentifier()
{
    auto identifierNode = std::make_shared<ASTNode>(ASTNodeType::Identifier,mLexer.line(), mLexer.column(), mLexer.token());

    if (handleIdentifierCall(identifierNode))
        return identifierNode;

    handleIdentifierAccess(identifierNode);

    mLexer.nextToken();

    if (mLexer.token() == ":=") {
        auto assignmentNode = std::make_shared<ASTNode>(ASTNodeType::Assignment,mLexer.line(), mLexer.column(), mLexer.token());

        if (!mLexer.nextToken())
            throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

        auto expressionNode = parseExpression();
        if (!expressionNode)
            return NdaParser::ASTNodePtr();

        ASTNode::addChild(assignmentNode,identifierNode);
        ASTNode::addChild(assignmentNode,expressionNode);

        return assignmentNode;

    } else {
        throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
    }
    return identifierNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseProcedureOrFunction()

//                         for procedures AND functions

{
    bool isFunction = mLexer.token() == "function";

    auto procedureNode = std::make_shared<ASTNode>(isFunction ? ASTNodeType::Function :
                                                                ASTNodeType::Procedure,
                                                    mLexer.line(), mLexer.column(),
                                                    mLexer.token());

    if (!mLexer.nextToken())
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    if (mLexer.tokenType() != NdaLexer::TokenType::Identifier)
        throw NdaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

    bool isMethod = mLexer.token(1) == ":";

    if (isMethod) {
        auto typeNode = std::make_shared<ASTNode>(ASTNodeType::MethodContext, mLexer.line(), mLexer.column(), mLexer.token());
        ASTNode::addChild(procedureNode,typeNode);
        mLexer.nextToken();      // to ":"
        if (!mLexer.nextToken()) // to method-name
            throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());
    }
    procedureNode->value = mLexer.token();

    if (!mLexer.nextToken())
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    auto parameterNode = parseFormalParameterList();
    ASTNode::addChild(procedureNode,parameterNode);

    if (isFunction) {
        // function Add(a : in Natural; b : in Natural) return Natural is
        //                                                |       |
        if (mLexer.token() != "return")
            throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
        mLexer.nextToken();

        if (mLexer.tokenType() != NdaLexer::TokenType::Identifier)
            throw NdaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

        auto returnNode = std::make_shared<ASTNode>(ASTNodeType::ReturnType, mLexer.line(), mLexer.column(), mLexer.token());
        ASTNode::addChild(procedureNode,returnNode);

        mLexer.nextToken();
    }

    if (mLexer.token() != "is")
        throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

    if (!mLexer.nextToken())
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());


    if (mLexer.token() != "begin")
        throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
    mLexer.nextToken();

    auto blockNode = std::make_shared<ASTNode>(ASTNodeType::Block, mLexer.line(), mLexer.column());
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
NdaParser::ASTNodePtr NdaParser::parseWhileLoop()
{
    auto whileNode = std::make_shared<ASTNode>(ASTNodeType::WhileLoop, mLexer.line(), mLexer.column());
    if (!mLexer.nextToken())
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    auto condition = parseExpression();

    if (!condition) // hmm assert?
        throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column(),mLexer.token());

    ASTNode::addChild(whileNode,condition);

    mLexer.nextToken();
    if (mLexer.token() != "loop")
        throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
    mLexer.nextToken();

    // 3. Parse den Block (Statements zwischen 'loop' und 'end loop')
    auto blockNode = std::make_shared<ASTNode>(ASTNodeType::Block, mLexer.line(), mLexer.column());
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
NdaParser::ASTNodePtr NdaParser::parseForLoop()
{
    auto forNode = std::make_shared<ASTNode>(ASTNodeType::ForLoop, mLexer.line(), mLexer.column());

    // consume "for"
    if (!mLexer.nextToken())
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    if (mLexer.tokenType() != NdaLexer::TokenType::Identifier)
        throw NdaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

    std::string loopVar = mLexer.token();

    if (!mLexer.nextToken())
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    if (mLexer.token() != "in")
        throw NdaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"in");

    if (!mLexer.nextToken())
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    auto iterableOrRangeNode = parseIterableOrRange();
    if (!iterableOrRangeNode)
        throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    mLexer.nextToken();
    if (mLexer.token() != "loop")
        throw NdaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"loop");

    mLexer.nextToken(); // Consume "loop"

    auto bodyNode = parseBlockEnd("end");
    if (!bodyNode)
        throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    if (mLexer.token() != "end")
        throw NdaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"end");
    mLexer.nextToken(); // Consume "end"

    if (mLexer.token() != "loop")
        throw NdaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"loop");

    // "loop" consumed by "parseSeparator()"

    forNode->value = loopVar;
    ASTNode::addChild(forNode,iterableOrRangeNode);
    ASTNode::addChild(forNode,bodyNode);

    return forNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseIfStatement()
{
    // Erwarte das Schlüsselwort "if"
    if (mLexer.token() != "if")
        throw NdaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"if");

    mLexer.nextToken(); // Consume "if"

    // 1. Parse die Bedingung
    auto conditionNode = parseExpression();
    if (!conditionNode) {
        throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());
    }

    // Erwarte "then"
    mLexer.nextToken();
    if (mLexer.token() != "then")
        throw NdaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(),"then");

    mLexer.nextToken(); // Consume "then"

    // Erstelle den IfStatement-Knoten
    auto ifNode = std::make_shared<ASTNode>(ASTNodeType::IfStatement, mLexer.line(), mLexer.column());
    ASTNode::addChild(ifNode,conditionNode);

    // 2. Parse den "then"-Block
    auto thenBlock = parseBlockEnd("elsif", "else");
    if (!thenBlock)
        throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    ASTNode::addChild(ifNode,thenBlock);

    // 3. Optional: Parse "elsif"-Blöcke
    while (mLexer.token() == "elsif") {
        mLexer.nextToken(); // Consume "elsif"
        auto elsifCondition = parseExpression();
        if (!elsifCondition) // assert?
            throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

        mLexer.nextToken();
        if (mLexer.token() != "then")
            throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

        mLexer.nextToken(); // Consume "then"

        auto elsifBlock = parseBlockEnd("elsif", "else");
        if (!elsifBlock)
            throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

        auto elsifNode = std::make_shared<ASTNode>(ASTNodeType::Elsif, mLexer.line(), mLexer.column());
        ASTNode::addChild(elsifNode,elsifCondition);
        ASTNode::addChild(elsifNode,elsifBlock);
        ASTNode::addChild(ifNode,elsifNode);
    }

    // 4. Optional: Parse "else"-Block
    if (mLexer.token() == "else") {
        mLexer.nextToken(); // Consume "else"

        auto elseBlock = parseBlockEnd("end");
        if (!elseBlock)
            throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

        auto elseNode = std::make_shared<ASTNode>(ASTNodeType::Else, mLexer.line(), mLexer.column());
        ASTNode::addChild(elseNode,elseBlock);
        ASTNode::addChild(ifNode,elseNode);
    }

    // 5. Erwarte "end if"
    if (mLexer.token() != "end")
        throw NdaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(), "end");

    if (!mLexer.nextToken()) // Consume "end"
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    if (mLexer.token() != "if")
        throw NdaException(Nada::Error::KeywordExpected,mLexer.line(), mLexer.column(), "if");
    return ifNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseReturn()
{
    auto returnNode = std::make_shared<ASTNode>(ASTNodeType::Return, mLexer.line(), mLexer.column());

    if (mLexer.token(1) != ";") { // optional expression present?

        if (!mLexer.nextToken())
                throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

        auto expression = parseExpression();

        if (!expression) // assert?
            throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

        ASTNode::addChild(returnNode,expression);
    }

    return returnNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseBreak()
//             break [when expression]
{
    auto breakNode = std::make_shared<ASTNode>(ASTNodeType::Break, mLexer.line(), mLexer.column());
    if (mLexer.token(1) != "when")
        return breakNode;

    mLexer.nextToken();        // step to   "when"
    if (!mLexer.nextToken())   // step over "when
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    auto expression = parseExpression();

    if (!expression) // assert?
        throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    ASTNode::addChild(breakNode,expression);
    return breakNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseContinue()
{
    auto contNode = std::make_shared<ASTNode>(ASTNodeType::Continue, mLexer.line(), mLexer.column());

    if (mLexer.token(1) != "when")
        return contNode;

    mLexer.nextToken();        // step to   "when"
    if (!mLexer.nextToken())   // step over "when
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    auto expression = parseExpression();

    if (!expression) // assert?
        throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column());

    ASTNode::addChild(contNode,expression);

    return contNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseBlockEnd(const std::string &endToken1, const std::string &endToken2)
{
    auto blockNode = std::make_shared<ASTNode>(ASTNodeType::Block, mLexer.line(), mLexer.column());

    while (mLexer.token() != "end" && mLexer.token() != endToken1 && (endToken2.empty() || mLexer.token() != endToken2)) {
        auto statementNode = parseStatement();
        if (!statementNode) {
            throw NdaException(Nada::Error::UnexpectedStructure,mLexer.line(), mLexer.column(),mLexer.token());
        }
        if (mLexer.token() != ";")
            throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

        ASTNode::addChild(blockNode,statementNode);
        if (!mLexer.nextToken())
            throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());
    }
    return blockNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseSeparator(const std::shared_ptr<ASTNode> &currentNode)
{
    if (!currentNode)
        return currentNode;

    if (!mLexer.nextToken())
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    if (mLexer.token() != ";")
        throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

    return currentNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseFormalParameterList()
{
    auto parametersNode = std::make_shared<ASTNode>(ASTNodeType::FormalParameters, mLexer.line(), mLexer.column());

    if (mLexer.token() != "(")
        return parametersNode;

    if (!mLexer.nextToken())
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

    while (mLexer.token() != ")") {

        if (mLexer.tokenType() != NdaLexer::TokenType::Identifier)
            throw NdaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

        std::string paramName = mLexer.token();
        mLexer.nextToken(); // Consume name

        if (mLexer.token() != ":")
            throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
        if (!mLexer.nextToken()) // Consume ":"
            throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column());

        std::string paramMode; // "in" / "out"
        if (mLexer.tokenType() == NdaLexer::TokenType::Keyword) {
            paramMode = mLexer.token();
            mLexer.nextToken();
        }

        // Parameter Type e.g. "Natural",..
        if (mLexer.tokenType() != NdaLexer::TokenType::Identifier)
            throw NdaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

        std::string paramType = mLexer.token();

        auto parameterNode = std::make_shared<ASTNode>(ASTNodeType::FormalParameter, mLexer.line(), mLexer.column(), paramName);
        auto parameterType = std::make_shared<ASTNode>(ASTNodeType::Identifier,      mLexer.line(), mLexer.column(), paramType);
        ASTNode::addChild(parameterNode,parameterType);

        if (!paramMode.empty()) {
            auto parameterMode = std::make_shared<ASTNode>(ASTNodeType::FormalParameterMode, mLexer.line(), mLexer.column(), paramMode);
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
std::string NdaParser::nodeTypeToString(ASTNodeType type)
{
    switch (type) {
    case ASTNodeType::Program:      return "Program";
    case ASTNodeType::WithAddon:    return "WithAddon";
    case ASTNodeType::Procedure:    return "Procedure";
    case ASTNodeType::Function:     return "Function";
    case ASTNodeType::FormalParameters:  return "Parameters";
    case ASTNodeType::FormalParameter :  return "Parameter";
    case ASTNodeType::MethodContext :    return "MethodContext";
    case ASTNodeType::Declaration:  return "Declaration";
    case ASTNodeType::VolatileDeclaration:  return "Volatile";
    case ASTNodeType::Assignment:   return "Assignment";
    case ASTNodeType::Expression:   return "Expression";
    case ASTNodeType::ExpressionList: return "ExpressionList";
    case ASTNodeType::Literal:      return "Literal";
    case ASTNodeType::ListLiteral:  return "ListLiteral";
    case ASTNodeType::Number:       return  "Number";
    case ASTNodeType::Identifier:   return "Identifier";
    case ASTNodeType::UnaryOperator:  return "UnaryOperator";
    case ASTNodeType::BinaryOperator: return "BinaryOperator";
    case ASTNodeType::FunctionCall:   return "FunctionCall";
    case ASTNodeType::StaticMethodCall:   return "StaticMethodCall";
    case ASTNodeType::InstanceMethodCall: return "InstanceMethodCall";
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
NdaParser::ASTNode::~ASTNode()
{
    if (variantCache)
        delete variantCache;
}

std::string NdaParser::ASTNode::serialize(int depth) const
{
    std::string indent(depth * 2, ' '); // Einrückung
    std::string result = indent + "Node(" + nodeTypeToString(type) + ", \"" + value.displayValue + "\")\n";
    for (const auto& child : children) {
        result += child->serialize(depth + 1);
    }
    return result;
}

//-------------------------------------------------------------------------------------------------
void NdaParser::ASTNode::addChild(std::shared_ptr<ASTNode> parent, std::shared_ptr<ASTNode> child)
{
    parent->children.push_back(child);
    child->parent = parent.get();
}

//-------------------------------------------------------------------------------------------------
void NdaParser::ASTNode::prependChild(std::shared_ptr<ASTNode> parent, std::shared_ptr<ASTNode> child)
{
    parent->children.insert(parent->children.begin(),child);
    child->parent = parent.get();
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseExpression()
{
    auto left = parseSimpleExpression();

    while (mLexer.token(1) == "and" || mLexer.token(1) == "or" || mLexer.token(1) == "xor") {
        mLexer.nextToken();
        auto operatorNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperator, mLexer.line(), mLexer.column(), mLexer.token());
        ASTNode::addChild(operatorNode,left);
        mLexer.nextToken(); // Hole den Operator
        auto right = parseSimpleExpression();
        ASTNode::addChild(operatorNode,right);
        left = operatorNode;
    }

    return left;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseSimpleExpression()
{
    std::string token;
    NdaLexer::TokenType tokenType;

    if (!mLexer.token(token,tokenType))
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    bool hasUnaryOperator = (token == "+" || token == "-");
    std::string unaryOperator = token;

    if (hasUnaryOperator)
        mLexer.nextToken();

    auto left = parseTerm();

    if (hasUnaryOperator) {
        auto term = left;
        left = std::make_shared<ASTNode>(ASTNodeType::UnaryOperator, mLexer.line(), mLexer.column(),unaryOperator);
        ASTNode::addChild(left,term);
    }

    while (mLexer.token(1) == "="  || mLexer.token(1) == "<>" ||
           mLexer.token(1) == "+"  || mLexer.token(1) == "-"  || mLexer.token(1) == "&" ||
           mLexer.token(1) == "<"  || mLexer.token(1) == ">"  ||
           mLexer.token(1) == "<=" || mLexer.token(1) == ">=")
    {
        mLexer.nextToken();
        auto operatorNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperator, mLexer.line(), mLexer.column(), mLexer.token());
        ASTNode::addChild(operatorNode,left);
        mLexer.nextToken(); // Hole den Operator
        auto right = parseTerm();
        ASTNode::addChild(operatorNode,right);
        left = operatorNode;
    }

    return left;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseTerm()
{
    auto left = parseFactor();

    while (mLexer.token(1) == "*" || mLexer.token(1) == "/" || mLexer.token(1) == "mod" || mLexer.token(1) == "rem") {
        mLexer.nextToken();
        auto operatorNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperator, mLexer.line(), mLexer.column(), mLexer.token());
        ASTNode::addChild(operatorNode,left);
        mLexer.nextToken(); // Hole den Operator
        auto right = parseFactor();
        ASTNode::addChild(operatorNode,right);
        left = operatorNode;
    }

    return left;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseFactor()
{
    auto left = parsePrimary();
    if (mLexer.token(1) == "**") {
        mLexer.nextToken();
        auto operatorNode = std::make_shared<ASTNode>(ASTNodeType::BinaryOperator, mLexer.line(), mLexer.column(), mLexer.token());
        ASTNode::addChild(operatorNode,left);
        mLexer.nextToken(); // Hole den Potenzierungsoperator
        auto right = parsePrimary();
        ASTNode::addChild(operatorNode,right);
        return operatorNode;
    }

    return left;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parsePrimary()
{
    std::string token;
    NdaLexer::TokenType tokenType;

    if (!mLexer.token(token,tokenType))
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    bool hasUnaryOperator = (token == "+" || token == "-" || token == "#");
    std::string unaryOperator = token;

    if (hasUnaryOperator)
        mLexer.nextToken();

    if (!mLexer.token(token,tokenType))
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    auto node = NdaParser::ASTNodePtr();

    if (tokenType == NdaLexer::TokenType::Number) {
        node = std::make_shared<ASTNode>(ASTNodeType::Number, mLexer.line(), mLexer.column(), token);
    } else if (tokenType == NdaLexer::TokenType::BooleanLiteral) {
        node = std::make_shared<ASTNode>(ASTNodeType::BooleanLiteral, mLexer.line(), mLexer.column(), token);
    }else if (tokenType == NdaLexer::TokenType::String) {
        node = std::make_shared<ASTNode>(ASTNodeType::Literal, mLexer.line(), mLexer.column(), token);
    } else if (tokenType == NdaLexer::TokenType::Identifier) {
        node = std::make_shared<ASTNode>(ASTNodeType::Identifier, mLexer.line(), mLexer.column(), token);

        if (!handleIdentifierCall(node))   // call()  ?
            handleIdentifierAccess(node);  // array[] ?

     } else if (token == "[") {
        node = parseListLiteral();
    } else if (token == "(") {
        if (!mLexer.nextToken()) // Überspringe '('
            throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

        node = std::make_shared<ASTNode>(ASTNodeType::Expression, mLexer.line(), mLexer.column());
        auto expression = parseExpression();
        ASTNode::addChild(node,expression);

        if (!mLexer.nextToken())
            throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

        if (mLexer.token() != ")") {
            throw NdaException(Nada::Error::UnexpectedClosure,mLexer.line(), mLexer.column(),mLexer.token());
        }
    } else
        throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),token);

    if (hasUnaryOperator) {
        // hmm: runtime error? Bool or Strings can't have unary operators!
        auto term = node;
        node = std::make_shared<ASTNode>(ASTNodeType::UnaryOperator, mLexer.line(), mLexer.column(),unaryOperator);
        ASTNode::addChild(node,term);
    }

    return node;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseFunctionCall(std::shared_ptr<ASTNode> &funcNode)
{
    mLexer.nextToken(); // jump to "("

    if (!mLexer.nextToken()) // jump over '('
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

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
            throw NdaException(Nada::Error::UnexpectedClosure,mLexer.line(), mLexer.column(),mLexer.token());
        }
    }

    return funcNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseMethodCall(std::shared_ptr<ASTNode> &funcNode)
{
    mLexer.nextToken(); // jump to ":" or "."
    bool isStatic = funcNode->type == ASTNodeType::StaticMethodCall;

    if (!mLexer.nextToken())    // Überspringe ':' oder '.'
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    if (mLexer.tokenType() != NdaLexer::TokenType::Identifier)
        throw NdaException(Nada::Error::IdentifierExpected,mLexer.line(), mLexer.column(),mLexer.token());

    if (mLexer.token(1) != "(") // lookahead: methodencall?
        throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());

    Nda::LowerString contextName = funcNode->value; // typename or instancename
    Nda::LowerString instanceName(mLexer.token());

    parseFunctionCall(funcNode);
    funcNode->value = instanceName;
    ASTNode::prependChild(funcNode,std::make_shared<ASTNode>(ASTNodeType::MethodContext, mLexer.line(), mLexer.column(), contextName.displayValue));

    return funcNode;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseIterableOrRange()
{
    if (mLexer.tokenType() == NdaLexer::TokenType::Number || mLexer.tokenType()  == NdaLexer::TokenType::Identifier) {
        auto start = parseExpression();

        if (mLexer.token(1) == "..") {
            mLexer.nextToken(); // step to   ".."
            mLexer.nextToken(); // step over ".."
            auto end = parseExpression();
            auto rangeNode = std::make_shared<ASTNode>(ASTNodeType::Range, mLexer.line(), mLexer.column());
            rangeNode->children.push_back(start);
            rangeNode->children.push_back(end);
            return rangeNode;
        }
        return start; // Es ist eine Iterable (z.B. anArray)
    }

    throw NdaException(Nada::Error::InvalidRangeOrIterable,mLexer.line(), mLexer.column(),mLexer.token());
    return NdaParser::ASTNodePtr();
}

//-------------------------------------------------------------------------------------------------
bool NdaParser::handleIdentifierCall(ASTNodePtr &identNode)
{
    if (mLexer.token(1) == "(") {
        identNode->type = ASTNodeType::FunctionCall;
        parseFunctionCall(identNode);
        return true;
    } else if (mLexer.token(1) == ":") {
        identNode->type = ASTNodeType::StaticMethodCall;
        parseMethodCall(identNode);
        return true;
    }
    else if (mLexer.token(1) == ".") {
        identNode->type = ASTNodeType::InstanceMethodCall;
        parseMethodCall(identNode);
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
bool NdaParser::handleIdentifierAccess(ASTNodePtr &identNode)
{
    if (mLexer.token(1) != "[")
        return false;

    auto accessNode = std::make_shared<ASTNode>(ASTNodeType::AccessOperator, mLexer.line(), mLexer.column());
    ASTNode::addChild(accessNode,identNode);

    if (!mLexer.nextToken())    // jump to "["
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    if (!mLexer.nextToken())    // jump over "["
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    auto index = parseExpression(); // [parse-this]
    ASTNode::addChild(accessNode,index);

    if (!mLexer.nextToken())    // jump to "]"
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    if (mLexer.token() != "]") {
        throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
    }


    identNode = accessNode;
    return true;
}

//-------------------------------------------------------------------------------------------------
NdaParser::ASTNodePtr NdaParser::parseListLiteral()
{
    assert(mLexer.token() == "[");

    auto ret = std::make_shared<ASTNode>(ASTNodeType::ListLiteral, mLexer.line(), mLexer.column());

    if (!mLexer.nextToken())    // Überspringe '['
        throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

    while (mLexer.token() != "]") {
        // Parse ein Element der Liste
        auto element = parseExpression();

        if (element)
            ASTNode::addChild(ret,element);

        if (!mLexer.nextToken())    // Überspringe '['
            throw NdaException(Nada::Error::UnexpectedEof,mLexer.line(), mLexer.column(),mLexer.token());

        // Optionales Komma
        if (mLexer.token() == ",") {
            mLexer.nextToken(); // Consume ','
        } else if (mLexer.token() != "]") {
            throw NdaException(Nada::Error::InvalidToken,mLexer.line(), mLexer.column(),mLexer.token());
        }
    }

    return ret;
}


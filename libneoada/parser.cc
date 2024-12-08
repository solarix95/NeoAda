#include <iostream>
#include "parser.h"

//-------------------------------------------------------------------------------------------------
NadaParser::NadaParser(NadaLexer &lexer)
    : mLexer(lexer)
    , mState(ParserState::None)
{
    mLexer.setLookAhead(1);
}

//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::parse(const std::string &script) {
    auto programNode = std::make_shared<ASTNode>(ASTNodeType::Program);

    mCurrentNode = programNode;
    mState = ParserState::ParsingProgram;

    mLexer.setScript(script);

    std::string token;
    NadaLexer::TokenType tokenType;

    while (mLexer.nextToken(token,tokenType)) {

        switch (mState) {
        case ParserState::ParsingProgram:
            if (tokenType == NadaLexer::TokenType::Keyword && token == "declare") {
                mState = ParserState::ParsingDeclaration;
                auto declarationNode = std::make_shared<ASTNode>(ASTNodeType::Declaration,mCurrentNode);
                ASTNode::addChild(mCurrentNode,declarationNode);
                mCurrentNode = declarationNode;
            } else if (tokenType == NadaLexer::TokenType::Identifier) {
                mState = ParserState::ParsingIdentifier;
                auto identifierNode = std::make_shared<ASTNode>(ASTNodeType::Identifier,token, mCurrentNode);
                mCurrentNode->children.push_back(identifierNode);
                mCurrentNode = identifierNode;
            } else {
                // Fehler: Unerwartetes Token
                onError("Unexpected token: " + token);
            }
            break;

        case ParserState::ParsingDeclaration:
            if (tokenType == NadaLexer::TokenType::Identifier) {
                auto currentNode = mCurrentNode;
                mCurrentNode->value = token; // Setze den Variablennamen
                mState = ParserState::ExpectingTypeSeparator; // Deklaration abgeschlossen

                // Erwarte ":"
                mLexer.nextToken(token,tokenType);
                if (token != ":")
                    return onError("Expected ':' after identifier, got: " + token);

                // Erwarte "Typ"
                mLexer.nextToken(token,tokenType);
                if (tokenType != NadaLexer::TokenType::Identifier)
                    return onError("Expected type after ':', got: " + token);

                auto typeNode = std::make_shared<ASTNode>(ASTNodeType::Identifier, token);
                ASTNode::addChild(mCurrentNode,typeNode);

                // Erwarte ";" oder ":="
                if (mLexer.token(1) == ":=") {
                    mLexer.nextToken();       // springe zu   ":="
                    if (!mLexer.nextToken())  // springe nach ":="
                        return onError("Expression expected");

                    auto expressionNode = parseExpression();
                    if (!expressionNode)
                        return std::shared_ptr<NadaParser::ASTNode>();

                    mCurrentNode->children.push_back(expressionNode);
                    // FIXME Error-Handling?
                }
                mState = ParserState::ExpectingSeparator;
                mCurrentNode = currentNode->parent;
            }
            break;

        // "x()" oder "x := "
        case ParserState::ParsingIdentifier:
            if (token == ":=") {
                mCurrentNode->type = ASTNodeType::Assignment;

                if (!mLexer.nextToken())
                    return onError("Expression expected");

                auto expressionNode = parseExpression();
                if (!expressionNode)
                    return std::shared_ptr<NadaParser::ASTNode>();
                ASTNode::addChild(mCurrentNode,expressionNode);

            } else if (token == "(") {
                parseFunctionCall(mCurrentNode);
            }
            mState = ParserState::ExpectingSeparator;
            break;
        case ParserState::ExpectingSeparator:
            if (mLexer.token() != ";")
              return onError("Separator ';' expected, got '" + mLexer.token() + "'");
            mState = ParserState::ParsingProgram;
            break;
        }
    }
    return programNode;
}


//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::onError(const std::string &msg)
{
    std::cerr << msg << std::endl;
    return std::shared_ptr<NadaParser::ASTNode>();
}

//-------------------------------------------------------------------------------------------------
void NadaParser::stepBack()
{
    std::cerr << "NOT IMPLEMENTED" << std::endl;
    /*
    switch (mCurrentNode->type) {
    case ASTNodeType::FunctionCall:
        switch (mCurrentNode->parent->type) {
        case ASTNodeType::Expression:

        }
    }
    */


}

//-------------------------------------------------------------------------------------------------
void NadaParser::stepBack(NadaLexer::TokenType currentTokenType, const std::string &token)
{
    switch (mCurrentNode->type) {
    case ASTNodeType::ExpressionList:
        if (token == ")") { // functioncall beendet
            mCurrentNode = mCurrentNode->parent;
            stepBack();
        }
    }
}

//-------------------------------------------------------------------------------------------------
std::string NadaParser::nodeTypeToString(ASTNodeType type)
{
    switch (type) {
    case ASTNodeType::Program: return "Program";
    case ASTNodeType::Declaration: return "Declaration";
    case ASTNodeType::Assignment: return "Assignment";
    case ASTNodeType::Expression: return "Expression";
    case ASTNodeType::ExpressionList: return "ExpressionList";
    case ASTNodeType::Literal: return "Literal";
    case ASTNodeType::Number: return "Number";
    case ASTNodeType::Identifier: return "Identifier";
    case ASTNodeType::BinaryOperator: return "BinaryOperator";
    case ASTNodeType::FunctionCall: return "FunctionCall";
    case ASTNodeType::IfStatement: return "IfStatement";
    case ASTNodeType::Loop: return "Loop";
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
    std::string result = indent + "Node(" + nodeTypeToString(type) + ", \"" + value + "\")\n";
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
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseExpression()
{
    auto leftOperand = parseSimpleExpression();
    return leftOperand;
}

//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseSimpleExpression()
{
    std::string token;
    NadaLexer::TokenType tokenType;

    if (!mLexer.token(token,tokenType))
        return onError("Simple expression expected");

    bool hasUnaryOperator = (token == "+" || token == "-");
    std::string unaryOperator = token;

    if (hasUnaryOperator)
        mLexer.nextToken();

    auto left = parseTerm();

    while (mLexer.token() == "+" || mLexer.token() == "-" || mLexer.token() == "&") {
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
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseTerm()
{
    auto left = parseFactor();

    while (mLexer.token() == "*" || mLexer.token() == "/" || mLexer.token() == "mod" || mLexer.token() == "rem") {
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
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseFactor()
{
    auto left = parsePrimary();

    if (mLexer.token() == "**") {
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
std::shared_ptr<NadaParser::ASTNode> NadaParser::parsePrimary()
{
    std::string token;
    NadaLexer::TokenType tokenType;

    if (!mLexer.token(token,tokenType))
        return onError("Primary expression expected");

    if (tokenType == NadaLexer::TokenType::Number) {
        auto node = std::make_shared<ASTNode>(ASTNodeType::Number, token);
        mLexer.nextToken();
        return node;
    } else if (tokenType == NadaLexer::TokenType::String) {
        auto node = std::make_shared<ASTNode>(ASTNodeType::Literal, token);
        mLexer.nextToken();
        return node;
    } else if (tokenType == NadaLexer::TokenType::Identifier) {
        auto identifier = std::make_shared<ASTNode>(ASTNodeType::Identifier, token);
        mLexer.nextToken();

        if (mLexer.token() == "(") {
            return parseFunctionCall(identifier);
        }
        return identifier;
    } else if (token == "(") {
        mLexer.nextToken(); // Überspringe '('
        auto expresionNode = std::make_shared<ASTNode>(ASTNodeType::Expression);
        auto expression = parseExpression();
        ASTNode::addChild(expresionNode,expression);
        if (mLexer.token() != ")") {
            return onError("Expected ')' after expression");
        }
        mLexer.nextToken(); // Überspringe ')'
        return expresionNode;

    } else
        return onError("Unknown primary expression: " + token);

    return std::shared_ptr<NadaParser::ASTNode>();
}

//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseFunctionCall(std::shared_ptr<ASTNode> &funcNode)
{
    funcNode->type = ASTNodeType::FunctionCall;

    mLexer.nextToken(); // Überspringe '('

    if (mLexer.token() != ")") {
        while (true) {
            auto argument = parseExpression();
            ASTNode::addChild(funcNode,argument);
            if (mLexer.token() == ",") {
                mLexer.nextToken(); // Überspringe ','
            } else {
                break;
            }
        }

        if (mLexer.token() != ")") {
            throw std::runtime_error("Expected ')' after function arguments");
        }
    }

    mLexer.nextToken(); // Überspringe ')'
    return funcNode;
}


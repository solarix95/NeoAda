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
std::shared_ptr<NadaParser::ASTNode> NadaParser::onError(const std::string &msg)
{
    std::cerr << msg << std::endl;
    return std::shared_ptr<NadaParser::ASTNode>();
}

//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseStatement()
{
    if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "declare") {
        return parseSeparator(parseDeclaration());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "while") {
        return parseSeparator(parseWhileLoop());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "if") {
        return parseSeparator(parseIfStatement());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Keyword && mLexer.token() == "return") {
        return parseSeparator(parseReturn());
    } else if (mLexer.tokenType() == NadaLexer::TokenType::Identifier) {
        return parseSeparator(parseIdentifier());
    }

    return onError("Unsupported Statement");
}

//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseDeclaration()
{
    auto declarationNode = std::make_shared<ASTNode>(ASTNodeType::Declaration);

    mLexer.nextToken(); // skip "declare"

    if (mLexer.tokenType() != NadaLexer::TokenType::Identifier) // FIXME: Errorhandling
        return onError("Identifier expected after 'declare'");

    declarationNode->value = mLexer.token(); // set variable name

    mLexer.nextToken(); // skip identifier

    if (mLexer.token() != ":")
        return onError("':' expeted");

    mLexer.nextToken(); // skip ':'

    if (mLexer.tokenType() != NadaLexer::TokenType::Identifier) // FIXME: Errorhandling
        return onError("Identifier expected after ':'");

    auto typeNode = std::make_shared<ASTNode>(ASTNodeType::Identifier, mLexer.token());
    ASTNode::addChild(declarationNode,typeNode);

    // Erwarte ";" oder ":="
    if (mLexer.token(1) == ":=") {
        mLexer.nextToken();       // springe zu   ":="
        if (!mLexer.nextToken())  // springe nach ":="
            return onError("Expression expected");

        auto expressionNode = parseExpression();
        if (!expressionNode)
            return std::shared_ptr<NadaParser::ASTNode>();
        ASTNode::addChild(declarationNode,expressionNode);
    }

    return declarationNode;
}

//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseIdentifier()
{
    auto identifierNode = std::make_shared<ASTNode>(ASTNodeType::Identifier,mLexer.token());

    mLexer.nextToken();

    if (mLexer.token() == ":=") {
        identifierNode->type = ASTNodeType::Assignment;

        if (!mLexer.nextToken())
            return onError("Expression expected");

        auto expressionNode = parseExpression();
        if (!expressionNode)
            return std::shared_ptr<NadaParser::ASTNode>();
        ASTNode::addChild(identifierNode,expressionNode);

    } else if (mLexer.token() == "(") {
        identifierNode->type = ASTNodeType::FunctionCall;
        parseFunctionCall(identifierNode);
    } else {
        return onError("Invalid token after identifier");
    }
    return identifierNode;
}

//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseWhileLoop()
{
    auto whileNode = std::make_shared<ASTNode>(ASTNodeType::WhileLoop);
    if (!mLexer.nextToken())
        return onError("Unexpected end after 'while'");

    auto condition = parseExpression();

    if (!condition)
        return onError("Expression expected after 'while'");

    ASTNode::addChild(whileNode,condition);

    mLexer.nextToken();
    if (mLexer.token() != "loop") {
        return onError("Expected 'loop' after while condition, got: " + mLexer.token());
    }
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
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseIfStatement()
{
    // Erwarte das Schlüsselwort "if"
    if (mLexer.token() != "if") {
        return onError("Expected 'if', got: " + mLexer.token());
    }
    mLexer.nextToken(); // Consume "if"

    // 1. Parse die Bedingung
    auto conditionNode = parseExpression();
    if (!conditionNode) {
        return onError("Expected condition expression after 'if'");
    }

    // Erwarte "then"
    mLexer.nextToken();
    if (mLexer.token() != "then") {
        return onError("Expected 'then' after condition, got: " + mLexer.token());
    }
    mLexer.nextToken(); // Consume "then"

    // Erstelle den IfStatement-Knoten
    auto ifNode = std::make_shared<ASTNode>(ASTNodeType::IfStatement);
    ASTNode::addChild(ifNode,conditionNode);

    // 2. Parse den "then"-Block
    auto thenBlock = parseBlockEnd("elsif", "else");
    if (!thenBlock)
        return onError("");
    ASTNode::addChild(ifNode,thenBlock);

    // 3. Optional: Parse "elsif"-Blöcke
    while (mLexer.token() == "elsif") {
        mLexer.nextToken(); // Consume "elsif"
        auto elsifCondition = parseExpression();
        if (!elsifCondition) {
            return onError("Expected condition expression after 'elsif'");
        }

        mLexer.nextToken();
        if (mLexer.token() != "then") {
            return onError("Expected 'then' after elsif condition, got: " + mLexer.token());
        }
        mLexer.nextToken(); // Consume "then"

        auto elsifBlock = parseBlockEnd("elsif", "else");
        if (!elsifBlock)
            return onError("");

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
            return onError("");

        auto elseNode = std::make_shared<ASTNode>(ASTNodeType::Else);
        ASTNode::addChild(elseNode,elseBlock);
        ASTNode::addChild(ifNode,elseNode);
    }

    // 5. Erwarte "end if"
    if (mLexer.token() != "end") {
        return onError("Expected 'end' to close 'if', got: " + mLexer.token());
    }
    mLexer.nextToken(); // Consume "end"

    if (mLexer.token() != "if") {
        return onError("Expected 'if' after 'end', got: " + mLexer.token());
    }

    return ifNode;
}

//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseReturn()
{
    auto returnNode = std::make_shared<ASTNode>(ASTNodeType::Return);
    if (!mLexer.nextToken())
        return onError("Unexpected end after 'while'");

    auto expression = parseExpression();

    if (!expression)
        return onError("Expression expected after 'return'");

    ASTNode::addChild(returnNode,expression);
    return returnNode;
}

//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseBlockEnd(const std::string &endToken1, const std::string &endToken2)
{
    auto blockNode = std::make_shared<ASTNode>(ASTNodeType::Block);

    while (mLexer.token() != "end" && mLexer.token() != endToken1 && (endToken2.empty() || mLexer.token() != endToken2)) {
        auto statementNode = parseStatement();
        if (!statementNode) {
            return onError("Invalid statement in 'then' block");
        }
        if (mLexer.token() != ";")
            return onError("';' expected");

        ASTNode::addChild(blockNode,statementNode);
        if (!mLexer.nextToken())
            return onError("Unexpected end in if-statement");
    }
    return blockNode;
}

//-------------------------------------------------------------------------------------------------
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseSeparator(const std::shared_ptr<ASTNode> &currentNode)
{
    if (!currentNode)
        return currentNode;

    if (!mLexer.nextToken())
        return onError("Unexpected end-of-script. Separator expected.");

    if (mLexer.token() != ";")
        return onError("Separator expected, got: " + mLexer.token());

    return currentNode;
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
    case ASTNodeType::UnaryOperator:  return "UnaryOperator";
    case ASTNodeType::BinaryOperator: return "BinaryOperator";
    case ASTNodeType::FunctionCall: return "FunctionCall";
    case ASTNodeType::IfStatement: return "If";
    case ASTNodeType::Else:  return "Else";
    case ASTNodeType::Elsif: return "ElseIf";
    case ASTNodeType::WhileLoop: return "WhileLoop";
    case ASTNodeType::Loop:   return "Loop";
    case ASTNodeType::Block:  return "Block";
    case ASTNodeType::Return: return "Return";
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

    if (hasUnaryOperator) {
        auto term = left;
        left = std::make_shared<ASTNode>(ASTNodeType::UnaryOperator,unaryOperator);
        ASTNode::addChild(left,term);
    }

    while (mLexer.token(1) == "=" || mLexer.token(1) == "/=" || mLexer.token(1) == "+" || mLexer.token(1) == "-" || mLexer.token(1) == "&" || mLexer.token(1) == "<" || mLexer.token(1) == ">") {
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
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseTerm()
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
std::shared_ptr<NadaParser::ASTNode> NadaParser::parseFactor()
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
std::shared_ptr<NadaParser::ASTNode> NadaParser::parsePrimary()
{
    std::string token;
    NadaLexer::TokenType tokenType;

    if (!mLexer.token(token,tokenType))
        return onError("Primary expression expected");

    if (tokenType == NadaLexer::TokenType::Number) {
        auto node = std::make_shared<ASTNode>(ASTNodeType::Number, token);
        return node;
    } else if (tokenType == NadaLexer::TokenType::BooleanLiteral) {
        auto node = std::make_shared<ASTNode>(ASTNodeType::BooleanLiteral, token);
        return node;
    }else if (tokenType == NadaLexer::TokenType::String) {
        auto node = std::make_shared<ASTNode>(ASTNodeType::Literal, token);
        return node;
    } else if (tokenType == NadaLexer::TokenType::Identifier) {
        auto identifier = std::make_shared<ASTNode>(ASTNodeType::Identifier, token);

        if (mLexer.token(1) == "(") {
            mLexer.nextToken();
            auto ret = parseFunctionCall(identifier);
            return ret;
        }
        return identifier;
    } else if (token == "(") {
        mLexer.nextToken(); // Überspringe '('
        auto expresionNode = std::make_shared<ASTNode>(ASTNodeType::Expression);
        auto expression = parseExpression();
        ASTNode::addChild(expresionNode,expression);

        mLexer.nextToken();
        if (mLexer.token() != ")") {
            return onError("Expected ')' after expression");
        }
        // mLexer.nextToken(); // Überspringe ')'
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
            mLexer.nextToken() ;
            if (mLexer.token() == ",") {
                mLexer.nextToken(); // jump over ","
            } else {
                break;
            }
        }

        if (mLexer.token() != ")") {
            throw std::runtime_error("Expected ')' after function arguments. Got " + mLexer.token(1));
        }
    }

    return funcNode;
}


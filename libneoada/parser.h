#ifndef LIB_NEOADA_PARSER_
#define LIB_NEOADA_PARSER_

#include <vector>
#include <memory>
#include "lexer.h"

class NadaParser
{
public:
    enum class ASTNodeType {
        Program,
        Declaration,
        Assignment,
        Expression,
        ExpressionList,
        Literal,
        Number,
        Identifier,
        BinaryOperator, // Für "+" "-" "*" "/"
        FunctionCall,
        IfStatement,
        Loop
    };

    enum class ParserState {
        None,           // Kein spezifischer Zustand
        ParsingProgram, // Ein Programm wird geparst
        ParsingDeclaration, // declare ..
        ParsingIdentifier,  // "x()" oder "x := "

        ExpectingTypeSeparator, ExpectingType,
        ExpectingPostIdentifier, // x ":=" oder x()
        ParsingExpression, ExpectingOperator, ExpectingExpressionPostIdentifier,
        ParsingExpressionList,
        ParsingIfStatement,
        ParsingLoop,

        ExpectingSeparator
    };

    struct ASTNode {        
        ASTNodeType type;
        std::string value; // Der Wert (z. B. Literal, Operator, Identifier)
        std::vector<std::shared_ptr<ASTNode>> children; // Unterknoten
        std::shared_ptr<ASTNode>              parent;

        ASTNode(ASTNodeType type, const std::string& value = "")
            : type(type), value(value) {}
        ASTNode(ASTNodeType type, std::shared_ptr<ASTNode> p, const std::string& value = "")
            : type(type), value(value), parent(p) {}
        ASTNode(ASTNodeType type, const std::string& value, std::shared_ptr<ASTNode> p)
            : type(type), value(value), parent(p) {}

        std::string serialize(int depth = 0) const;
        static void addChild(std::shared_ptr<ASTNode> parent, std::shared_ptr<ASTNode> child);
    };

    NadaParser(NadaLexer &lexer);

    std::shared_ptr<ASTNode> parse(const std::string &script);

private:
    std::shared_ptr<ASTNode> onError(const std::string &msg);
    std::shared_ptr<ASTNode> parseExpression();

    std::shared_ptr<ASTNode> parseSimpleExpression();   // a > b
    std::shared_ptr<ASTNode> parseTerm();               // a * b
    std::shared_ptr<ASTNode> parseFactor();             // a**b
    std::shared_ptr<ASTNode> parsePrimary();            // a
    std::shared_ptr<ASTNode> parseFunctionCall(std::shared_ptr<ASTNode> &funcNode);       // a()


    void stepBack();
    void stepBack(NadaLexer::TokenType currentTokenType, const std::string &token);


    static std::string nodeTypeToString(ASTNodeType type);

    NadaLexer               &mLexer;
    ParserState              mState;
    std::shared_ptr<ASTNode> mCurrentNode;
};



#endif
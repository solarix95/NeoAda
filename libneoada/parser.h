#ifndef LIB_NEOADA_PARSER_
#define LIB_NEOADA_PARSER_

#include <vector>
#include <memory>
#include "lexer.h"
#include "utils.h"

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
        BooleanLiteral,
        UnaryOperator,
        BinaryOperator, // Für "+" "-" "*" "/"
        FunctionCall,
        Block,          // Begin/End
        IfStatement,
        Else,
        Elsif,
        WhileLoop,
        ForLoop,
        Loop,
        Return,
        Break,
        Continue,
        Range,
    };

    enum class ParserState {
        None,           // Kein spezifischer Zustand
        ParsingProgram, // Ein Programm wird geparst
        ParsingDeclaration, // declare ..
        ParsingIdentifier,  // "x()" oder "x := "
        ParsingWhileLoop,

        ExpectingTypeSeparator, ExpectingType,
        ExpectingPostIdentifier, // x ":=" oder x()
        ParsingExpression, ExpectingOperator, ExpectingExpressionPostIdentifier,
        ParsingExpressionList,
        ParsingIfStatement,
        ParsingLoop,

        ExpectingSeparator
    };

    struct ASTNode {        
        ASTNodeType       type;
        Nada::LowerString value; // Der Wert (z. B. Literal, Operator, Identifier)
        std::vector<std::shared_ptr<ASTNode>> children; // Unterknoten
        std::shared_ptr<ASTNode>              parent;

        ASTNode(ASTNodeType type, const std::string& value = "")
            : type(type), value(value) {}
        ASTNode(ASTNodeType type, std::shared_ptr<ASTNode> p, const std::string& value = "")
            : type(type), value(value), parent(p){}
        ASTNode(ASTNodeType type, const std::string& value, std::shared_ptr<ASTNode> p)
            : type(type), value(value), parent(p) {}

        std::string serialize(int depth = 0) const;
        static void addChild(std::shared_ptr<ASTNode> parent, std::shared_ptr<ASTNode> child);
    };

    NadaParser(NadaLexer &lexer);

    std::shared_ptr<ASTNode> parse(const std::string &script);

private:
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseDeclaration();
    std::shared_ptr<ASTNode> parseIdentifier();  // "call()" or "var :="
    std::shared_ptr<ASTNode> parseWhileLoop();
    std::shared_ptr<ASTNode> parseForLoop();
    std::shared_ptr<ASTNode> parseIfStatement();
    std::shared_ptr<ASTNode> parseReturn();
    std::shared_ptr<ASTNode> parseBreak();
    std::shared_ptr<ASTNode> parseContinue();
    std::shared_ptr<ASTNode> parseBlockEnd(const std::string& endToken1,
                                           const std::string& endToken2 = "");   // von "if"/"else"/"elsif" bis "end"
    std::shared_ptr<ASTNode> parseSeparator(const std::shared_ptr<ASTNode> &currentNode);


    std::shared_ptr<ASTNode> parseExpression();

    std::shared_ptr<ASTNode> parseSimpleExpression();   // a > b
    std::shared_ptr<ASTNode> parseTerm();               // a * b
    std::shared_ptr<ASTNode> parseFactor();             // a**b
    std::shared_ptr<ASTNode> parsePrimary();            // a
    std::shared_ptr<ASTNode> parseFunctionCall(std::shared_ptr<ASTNode> &funcNode);       // a()
    std::shared_ptr<ASTNode> parseIterableOrRange();    // for x in [IterableOrRange]

    static std::string nodeTypeToString(ASTNodeType type);

    NadaLexer               &mLexer;
    ParserState              mState;
    std::shared_ptr<ASTNode> mCurrentNode;
};



#endif

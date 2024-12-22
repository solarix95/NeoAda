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
        Procedure,
        FormalParameters,    // procedure/function declaration: call(FormalParameters)
        FormalParameter,     // procedure/function declaration: call(FormalParameter, FormalParameter, FormalParameter)
        FormalParameterMode, // procedure/function declaration: call(x : [in/out] Natural)
        Function,
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

    using ASTNodePtr = std::shared_ptr<ASTNode>;

    NadaParser(NadaLexer &lexer);

    NadaParser::ASTNodePtr parse(const std::string &script);

private:
    NadaParser::ASTNodePtr parseStatement();
    NadaParser::ASTNodePtr parseDeclaration();
    NadaParser::ASTNodePtr parseIdentifier();  // "call()" or "var :="
    NadaParser::ASTNodePtr parseProcedure();
    NadaParser::ASTNodePtr parseWhileLoop();
    NadaParser::ASTNodePtr parseForLoop();
    NadaParser::ASTNodePtr parseIfStatement();
    NadaParser::ASTNodePtr parseReturn();
    NadaParser::ASTNodePtr parseBreak();
    NadaParser::ASTNodePtr parseContinue();

    // Parse Sub-Elements
    NadaParser::ASTNodePtr parseBlockEnd(const std::string& endToken1,
                                           const std::string& endToken2 = "");         // block/scope from "if"/"else"/"elsif" to "end"
    NadaParser::ASTNodePtr parseSeparator(const NadaParser::ASTNodePtr &currentNode);

    NadaParser::ASTNodePtr parseFormalParameterList();                                 // procedure/function parameters;


    NadaParser::ASTNodePtr parseExpression();

    NadaParser::ASTNodePtr parseSimpleExpression();   // a > b
    NadaParser::ASTNodePtr parseTerm();               // a * b
    NadaParser::ASTNodePtr parseFactor();             // a**b
    NadaParser::ASTNodePtr parsePrimary();            // a
    NadaParser::ASTNodePtr parseFunctionCall(NadaParser::ASTNodePtr &funcNode);       // a()
    NadaParser::ASTNodePtr parseIterableOrRange();    // for x in [IterableOrRange]

    static std::string nodeTypeToString(ASTNodeType type);

    NadaLexer               &mLexer;
    ParserState              mState;
    NadaParser::ASTNodePtr mCurrentNode;
};



#endif

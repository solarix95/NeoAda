#ifndef LIB_NEOADA_PARSER_
#define LIB_NEOADA_PARSER_

#include <vector>
#include <memory>
#include "lexer.h"
#include "private/utils.h"

class NadaParser
{
public:
    enum class ASTNodeType {
        Program,
        WithAddon,
        Procedure,
        FormalParameters,    // procedure/function declaration: call(FormalParameters)
        FormalParameter,     // procedure/function declaration: call(FormalParameter, FormalParameter, FormalParameter)
        FormalParameterMode, // procedure/function declaration: call(x : [in/out] Natural)
        Function,
        Declaration,
        MethodContext,
        VolatileDeclaration,
        Assignment,
        Expression,
        ExpressionList,
        Literal,
        ListLiteral,
        Number,
        Identifier,
        BooleanLiteral,
        AccessOperator,  // list/dict []
        UnaryOperator,
        BinaryOperator,  // Für "+" "-" "*" "/"
        FunctionCall,
        StaticMethodCall,
        InstanceMethodCall,
        Block,          // Begin/End
        IfStatement,
        Else,
        Elsif,
        WhileLoop,
        ForLoop,
        Loop,
        Return,
        ReturnType,
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
        Nda::LowerString  value; // Der Wert (z. B. Literal, Operator, Identifier)
        std::vector<std::shared_ptr<ASTNode>> children; // Unterknoten
        std::shared_ptr<ASTNode>              parent;

        int                                   line;
        int                                   column;

        ASTNode(ASTNodeType type, int l, int c, const std::string& value = "")
            : type(type), value(value), line(l), column(c) {}


        std::string serialize(int depth = 0) const;
        static void addChild(std::shared_ptr<ASTNode> parent, std::shared_ptr<ASTNode> child);
        static void prependChild(std::shared_ptr<ASTNode> parent, std::shared_ptr<ASTNode> child);
    };

    using ASTNodePtr = std::shared_ptr<ASTNode>;

    NadaParser(NadaLexer &lexer);

    NadaParser::ASTNodePtr parse(const std::string &script);

private:
    NadaParser::ASTNodePtr parseStatement();
    NadaParser::ASTNodePtr parseDeclaration();
    NadaParser::ASTNodePtr parseWith();
    NadaParser::ASTNodePtr parseIdentifier();  // "call()" or "var :="
    NadaParser::ASTNodePtr parseProcedureOrFunction();
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
    NadaParser::ASTNodePtr parseFunctionCall(NadaParser::ASTNodePtr &funcNode);        // a()
    NadaParser::ASTNodePtr parseMethodCall(NadaParser::ASTNodePtr &funcNode);          // type:a()
    NadaParser::ASTNodePtr parseIterableOrRange();    // for x in [IterableOrRange]
    bool                   handleIdentifierCall(NadaParser::ASTNodePtr &identNode);    // is function or procedure or method-call
    bool                   handleIdentifierAccess(NadaParser::ASTNodePtr &identNode);  // is array/dict access operator
    NadaParser::ASTNodePtr parseListLiteral();         // is function or procedure or method-call

    static std::string nodeTypeToString(ASTNodeType type);

    NadaLexer               &mLexer;
    ParserState              mState;
    NadaParser::ASTNodePtr mCurrentNode;
};



#endif

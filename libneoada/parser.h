#ifndef LIB_NEOADA_PARSER_
#define LIB_NEOADA_PARSER_

#include <vector>
#include <memory>
#include "lexer.h"
#include "private/utils.h"

class NdaVariant;
class NdaParser
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

    struct ASTNode {        
        ASTNodeType       type;
        Nda::LowerString  value; // Der Wert (z. B. Literal, Operator, Identifier)
        std::vector<std::shared_ptr<ASTNode>> children; // Unterknoten
        ASTNode                              *parent;

        int                                   line;
        int                                   column;
        NdaVariant                           *variantCache;

        ASTNode(ASTNodeType type, int l, int c, const std::string& value = "")
            : type(type), value(value), line(l), column(c), variantCache(nullptr) {}
        ~ASTNode();

        std::string serialize(int depth = 0) const;
        static void addChild(std::shared_ptr<ASTNode> parent, std::shared_ptr<ASTNode> child);
        static void prependChild(std::shared_ptr<ASTNode> parent, std::shared_ptr<ASTNode> child);
    };

    using ASTNodePtr = std::shared_ptr<ASTNode>;

    NdaParser(NdaLexer &lexer);

    NdaParser::ASTNodePtr parse(const std::string &script);

private:
    NdaParser::ASTNodePtr parseStatement();
    NdaParser::ASTNodePtr parseDeclaration();
    NdaParser::ASTNodePtr parseWith();
    NdaParser::ASTNodePtr parseIdentifier();  // "call()" or "var :="
    NdaParser::ASTNodePtr parseProcedureOrFunction();
    NdaParser::ASTNodePtr parseWhileLoop();
    NdaParser::ASTNodePtr parseForLoop();
    NdaParser::ASTNodePtr parseIfStatement();
    NdaParser::ASTNodePtr parseReturn();
    NdaParser::ASTNodePtr parseBreak();
    NdaParser::ASTNodePtr parseContinue();

    // Parse Sub-Elements
    NdaParser::ASTNodePtr parseBlockEnd(const std::string& endToken1,
                                           const std::string& endToken2 = "");         // block/scope from "if"/"else"/"elsif" to "end"
    NdaParser::ASTNodePtr parseSeparator(const NdaParser::ASTNodePtr &currentNode);

    NdaParser::ASTNodePtr parseFormalParameterList();                                 // procedure/function parameters;


    NdaParser::ASTNodePtr parseExpression();

    NdaParser::ASTNodePtr parseSimpleExpression();   // a > b
    NdaParser::ASTNodePtr parseTerm();               // a * b
    NdaParser::ASTNodePtr parseFactor();             // a**b
    NdaParser::ASTNodePtr parsePrimary();            // a
    NdaParser::ASTNodePtr parseFunctionCall(NdaParser::ASTNodePtr &funcNode);        // a()
    NdaParser::ASTNodePtr parseMethodCall(NdaParser::ASTNodePtr &funcNode);          // type:a()
    NdaParser::ASTNodePtr parseIterableOrRange();    // for x in [IterableOrRange]
    bool                   handleIdentifierCall(NdaParser::ASTNodePtr &identNode);    // is function or procedure or method-call
    bool                   handleIdentifierAccess(NdaParser::ASTNodePtr &identNode);  // is array/dict access operator
    NdaParser::ASTNodePtr parseListLiteral();         // is function or procedure or method-call

    static std::string nodeTypeToString(ASTNodeType type);

    NdaLexer               &mLexer;
    NdaParser::ASTNodePtr mCurrentNode;
};



#endif

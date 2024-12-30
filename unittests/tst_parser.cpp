#include "neoadaapi.h"
#include <QtTest>

#include <iostream>
#include <sstream>

#include <libneoada/exception.h>
#include <libneoada/lexer.h>
#include <libneoada/parser.h>
#include <libneoada/state.h>
#include <libneoada/interpreter.h>

#include <libneoada/private/sharedstring.h>


// add necessary includes here

// trim from start (in place)
inline std::string ltrim(const std::string &s) {
    std::string ret = s;
    ret.erase(ret.begin(), std::find_if(ret.begin(), ret.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
    return ret;
}

// trim from end (in place)
inline std::string rtrim(const std::string &s) {
    std::string ret = s;
    ret.erase(std::find_if(ret.rbegin(), ret.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), ret.end());
    return ret;
}

// trim from both ends (in place)
inline std::string trim(const std::string &s) {
    std::string ret = rtrim(s);
    return ltrim(ret);
}

#define QCOMPARE_TRIM(a,b)  if (trim(a)!=trim(b)) { std::cout <<  "A\n" << trim(a) << std::endl << "B\n" << trim(b) << std::endl; } QCOMPARE(trim(a),trim(b))

class TstParser : public QObject
{
    Q_OBJECT

public:
    TstParser();
    ~TstParser();

private slots:

    // Core Datastructure
    void test_core_SharedString();
    void test_core_NumericValues();
    void test_core_Assignment();
    void test_core_References();
    void test_core_List_COW();
    void test_core_List_REF();
    void test_core_List_Contains();
    void test_core_List_Concat();


    // Lexer Literals
    void test_lexer_Numbers();
    void test_lexer_OperatorsAndSymbols();
    void test_lexer_Identifiers();
    void test_lexer_Boolean();
    void test_lexer_Strings();
    void test_lexer_Range();
    void test_lexer_Comments();
    void test_lexer_Expression1();
    void test_lexer_Expression2();
    void test_lexer_HelloWorld();
    void test_lexer_HelloWorld2();

    void test_parser_Declaration1();
    void test_parser_Declaration2();
    void test_parser_Declaration3();
    void test_parser_Declaration4_List();
    void test_parser_Declaration5_List_Init();

    void test_parser_Factor();
    void test_parser_Primary1();
    void test_parser_Primary2();

    void test_parser_Relation1();
    void test_parser_Relation2();
    void test_parser_Relation3();

    void test_parser_SimpleExpression1();
    void test_parser_SimpleExpression2();
    void test_parser_SimpleExpression3();
    void test_parser_SimpleExpression4();
    void test_parser_SimpleExpression5();
    void test_parser_SimpleExpression6();

    void test_parser_Expression1();
    void test_parser_Expression2();

    void test_parser_Expression3();
    void test_parser_Expression4();
    void test_parser_Expression5();
    void test_parser_Expression6();
    void test_parser_Expression7();
    void test_parser_Expression8();
    void test_parser_Expression9();

    void test_parser_HelloWorld();
    void test_parser_HelloWorld2();

    void test_parser_SimpleProgram();
    void test_parser_SimpleExpression();

    void test_parser_FunctionCall1();
    void test_parser_MethodCall1();
    void test_parser_MethodCall2();

    void test_parser_WhileLoop();
    void test_parser_WhileLoopBreak1();
    void test_parser_WhileLoopBreak2();

    void test_parser_ForLoopRange();

    void test_parser_If();
    void test_parser_IfElse();
    void test_parser_ElseIf1();
    void test_parser_ElseIf2();
    void test_parser_ElseIf3();

    void test_parser_return();

    void test_parser_Procedure1();
    void test_parser_Procedure2();
    void test_parser_Function1();
    void test_parser_Method1();
    void test_parser_Method2();

    void test_state_Declarations();
    void test_state_GlobalScope();

    void test_interpreter_Declarations();
    void test_interpreter_ProcedureCall();
    void test_interpreter_ifStatement();
    void test_interpreter_ifElseStatement();
    void test_interpreter_whileStatement();
    void test_interpreter_whileBreak();
    void test_interpreter_whileBreakWhen();
    void test_interpreter_whileContinue();
    void test_interpreter_Return1();
    void test_interpreter_Return2();
    void test_interpreter_Return3();

    void test_interpreter_Volatile_CTor();

    void test_interpreter_static_method();


    void test_api_evaluate_Literals();
    void test_api_evaluate_Length();
    void test_api_evaluate_Equal();
    void test_api_evaluate_NotEqual();
    void test_api_evaluate_LessThan();
    void test_api_evaluate_LessEqualThan();

    void test_api_evaluate_ConcatString();
    void test_api_evaluate_Modulo();
    void test_api_evaluate_Multiply();
    void test_api_evaluate_Relations();

    void test_api_evaluate_List_Init();

    void test_api_evaluate_GlobalValue();
    void test_api_evaluate_ScopeValue();
    void test_api_evaluate_WhileBreak();
    void test_api_evaluate_WhileContinue();
    void test_api_evaluate_ForLoop();
    void test_api_evaluate_ForLoopBreak();
    void test_api_evaluate_ForLoopContinue();

    void test_api_evaluate_Procedure1();
    void test_api_evaluate_Procedure2();
    void test_api_evaluate_Procedure3_Return();
    void test_api_evaluate_Procedure4_Out();
    void test_api_evaluate_Function1();
    void test_api_evaluate_Function2();
    void test_api_evaluate_Function2_Uppercase();

    void test_api_evaluate_Static_Method1();
    void test_api_evaluate_Static_Method2();

    void test_api_evaluate_Instance_Method1();


    // static ERROR HANDLING
    void test_error_lexer_invalidCharacter();
    void test_error_lexer_invalidString();
    void test_error_lexer_invalidBasedLiteral();

    void test_error_parser_declaration1();
    void test_error_parser_declaration2();
    void test_error_parser_declaration3();
    void test_error_parser_declaration4();

    void test_error_parser_if1();
    void test_error_parser_ifElse1();

    // runtime ERROR HANDLING
    void test_error_interpreter_stringAssignment();
    void test_error_interpreter_boolAssignment();


};

TstParser::TstParser() {}

TstParser::~TstParser() {}

/*-----------------------------------------------------------------------------------------------*\
                                       CORE DATASTRUCTURES
\*-----------------------------------------------------------------------------------------------*/

void TstParser::test_core_SharedString()
{
    // Test "Copy on Write"
    NadaValue s1;

    QCOMPARE(s1.refCount(), 0);

    s1.fromString("");
    QCOMPARE(s1.refCount(), 0);
    QCOMPARE(s1.toString(), "");

    s1.fromString("1");
    QCOMPARE(s1.refCount(), 1);
    QCOMPARE(s1.toString(), "1");

    NadaValue s2(s1);
    QCOMPARE(s1.refCount(), 2);
    QCOMPARE(s2.refCount(), 2);
    QCOMPARE(s2.toString(), "1");

    // same value -> don't change situation
    s2.setString("1");
    QCOMPARE(s1.refCount(), 2);
    QCOMPARE(s2.refCount(), 2);
    QCOMPARE(s2.toString(), "1");


    // new value -> detach shared data
    s2.setString("2");
    QCOMPARE(s1.refCount(), 1);
    QCOMPARE(s2.refCount(), 1);
    QCOMPARE(s2.toString(), "2");

    s1.setString("");
    QCOMPARE(s1.refCount(), 0);
    QCOMPARE(s2.refCount(), 1);

    s2.setString("");
    QCOMPARE(s1.refCount(), 0);
    QCOMPARE(s2.refCount(), 0);

    s1.setString("1");

    {
        NadaValue s3;
        QCOMPARE(s1.refCount(), 1);
        QCOMPARE(s3.refCount(), 0);

        s3 = s1;
        QCOMPARE(s1.refCount(), 2);
        QCOMPARE(s3.refCount(), 2);
    }
    QCOMPARE(s1.refCount(), 1);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_NumericValues()
{
    NadaValue v;

    QCOMPARE(v.fromNumber("123_456"),true);      // int64_t
    QCOMPARE(v.type(), Nda::Natural);

    QCOMPARE(v.fromNumber("1.23E+10"),true);     // double
    QCOMPARE(v.type(), Nda::Number);

    QCOMPARE(v.fromNumber("16#1F#"),true);       // uint64_t (Base 16)
    QCOMPARE(v.type(), Nda::Supernatural);

    QCOMPARE(v.fromNumber("2#1011_0001#"),true); // uint64_t (Base 2)
    QCOMPARE(v.type(), Nda::Supernatural);

    QCOMPARE(v.fromNumber("10#123#E+2"),true);   // uint64_t mit Exponent
    QCOMPARE(v.type(), Nda::Supernatural);

    QCOMPARE(v.fromNumber("1.2_34"),true);       // double
    QCOMPARE(v.type(), Nda::Number);

    QCOMPARE(v.fromNumber("18446744073709551615"),true); // uint64_t (Maximalwert)
    QCOMPARE(v.type(), Nda::Supernatural);

    QCOMPARE(v.fromNumber("InvalidLiteral"),false);
    QCOMPARE(v.type(), Nda::Undefined);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_Assignment()
{
    NadaValue v1;
    NadaValue v2;


    // Number -> Any
    v1.initAny();
    v2.fromNumber(42.23);
    QCOMPARE(v1.assign(v2),true);

    // Natural -> Any
    v1.initAny();
    v2.fromNumber((int64_t)42);
    QCOMPARE(v1.assign(v2),true);

    // Supernatural -> Any
    v1.initAny();
    v2.fromNumber((uint64_t)42);
    QCOMPARE(v1.assign(v2),true);

    // String -> Any
    v1.initAny();
    v2.fromString("neoAda");
    QCOMPARE(v1.assign(v2),true);
    QCOMPARE(v1.toString(), v2.toString());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_References()
{
    NadaValue v1;
    NadaValue r1;
    r1.fromReference(&v1);


    v1.initAny();
    QCOMPARE(r1.type(), Nda::Type::Any);

    v1.fromString("hello");
    QCOMPARE(r1.type(), Nda::Type::String);
    QCOMPARE(r1.toString(), v1.toString());

    NadaValue r2 = r1; // copy reference
    NadaValue v2;
    v2.fromString("world");

    r2.assign(v2);

    QCOMPARE(v1.toString(), v2.toString());
    QCOMPARE(r1.toString(), v1.toString());
    QCOMPARE(r2.toString(), v1.toString());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_List_COW()
{
    NadaValue vs;
    vs.fromString("1");

    { // append, insert remove
        NadaValue a1;
        a1.initType(Nda::List);

        QCOMPARE(a1.listSize(), 0);
        a1.appendToList(vs);
        QCOMPARE(a1.listSize(), 1);
        QVERIFY(a1.readAccess(0).toString() == "1");

        vs.fromString("0");
        a1.insertIntoList(0,vs);
        QVERIFY(a1.readAccess(0).toString() == "0");
        QVERIFY(a1.readAccess(1).toString() == "1");
        a1.takeFromList(0);
        QVERIFY(a1.readAccess(0).toString() == "1");
        a1.takeFromList(0);
        QCOMPARE(a1.listSize(), 0);
    }

    { // shared list
        NadaValue a1;
        a1.initType(Nda::List);

        {
            NadaValue a2;
            a2.initType(Nda::List);

            vs.fromString("a2");
            a2.appendToList(vs);

            QCOMPARE(a1.listSize(), 0);
            QCOMPARE(a2.listSize(), 1);

            a1 = a2;
        } // destroy a2

        QCOMPARE(a1.listSize(), 1);
        QVERIFY(a1.readAccess(0).toString() == "a2");
    }

    { // copy-on-write: source changes
        NadaValue a1;
        a1.initType(Nda::List);

        NadaValue a2;
        a2.initType(Nda::List);

        vs.fromString("0");
        a1.appendToList(vs);

        a2 = a1;
        QVERIFY(a1.readAccess(0).toString() == "0");
        QVERIFY(a2.readAccess(0).toString() == "0");

        vs.fromString("1");
        a1.insertIntoList(0,vs);
        QVERIFY(a1.readAccess(0).toString() == "1");
        QVERIFY(a2.readAccess(0).toString() == "0");
        QCOMPARE(a1.listSize(), 2);
        QCOMPARE(a2.listSize(), 1);
    }

    { // copy-on-write: target changes
        NadaValue a1;
        a1.initType(Nda::List);

        NadaValue a2;
        a2.initType(Nda::List);

        vs.fromString("0");
        a1.appendToList(vs);

        a2 = a1;
        QVERIFY(a1.readAccess(0).toString() == "0");
        QVERIFY(a2.readAccess(0).toString() == "0");

        vs.fromString("1");
        a2.insertIntoList(0,vs);
        QVERIFY(a1.readAccess(0).toString() == "0");
        QVERIFY(a2.readAccess(0).toString() == "1");
        QCOMPARE(a1.listSize(), 1);
        QCOMPARE(a2.listSize(), 2);
    }
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_List_REF()
{
    NadaValue vs;
    vs.fromString("1");

    {
        NadaValue a1;
        a1.initType(Nda::List);
        a1.appendToList(vs);

        NadaValue r1;
        r1.fromReference(&a1);
        r1.appendToList(vs);

        a1.appendToList(vs);

        QCOMPARE(a1.listSize(), 3);
        QCOMPARE(r1.listSize(), 3);

        while (r1.listSize() > 0)
            r1.takeFromList(0);

        QCOMPARE(a1.listSize(), 0);
        QCOMPARE(r1.listSize(), 0);
    }
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_core_List_Contains()
{
    NadaValue vs;
    vs.fromString("0");

    NadaValue a1;
    a1.initType(Nda::List);
    a1.appendToList(vs);

    QVERIFY(a1.containsInList(vs) == true);

    vs.fromNumber((int64_t)0);
    QVERIFY(a1.containsInList(vs) == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_List_Concat()
{
    NadaValue vs;
    vs.fromString("0");

    NadaValue a1;
    a1.initType(Nda::List);
    a1.appendToList(vs);

    vs.fromString("1");
    NadaValue a2;
    a2.initType(Nda::List);
    a2.appendToList(vs);

    NadaValue a3 = a1.concat(a2);

    vs.fromString("0");
    QVERIFY(a3.containsInList(vs) == true);
    vs.fromString("1");
    QVERIFY(a3.containsInList(vs) == true);

    QVERIFY(a1.listSize() == 1);
    QVERIFY(a2.listSize() == 1);
    QVERIFY(a3.listSize() == 2);
}

/*-----------------------------------------------------------------------------------------------*\
                                       LEXER LITERALS
\*-----------------------------------------------------------------------------------------------*/


//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_Numbers()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("123 1_000.0 42E+3 16#FF# 2#1010#E+2");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 5);
    QVERIFY(results[0] == "123");
    QVERIFY(results[1] == "1_000.0");
    QVERIFY(results[2] == "42E+3");
    QVERIFY(results[3] == "16#FF#");
    QVERIFY(results[4] == "2#1010#E+2");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_OperatorsAndSymbols()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript(":= ** <= >= + - * / > < ( ) ;");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 13);
    QVERIFY(results[0] == ":=");
    QVERIFY(results[1] == "**"); // Ada95 Power
    QVERIFY(results[2] == "<=");
    QVERIFY(results[3] == ">=");

    QVERIFY(results[4] == "+");
    QVERIFY(results[5] == "-");
    QVERIFY(results[6] == "*");
    QVERIFY(results[7] == "/");
    QVERIFY(results[8] == ">");
    QVERIFY(results[9] == "<");
    QVERIFY(results[10] == "(");
    QVERIFY(results[11] == ")");
    QVERIFY(results[12] == ";");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_Identifiers()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("myVar _private x123");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 3);
    QVERIFY(results[0] == "myVar");
    QVERIFY(results[1] == "_private");
    QVERIFY(results[2] == "x123");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_Boolean()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("true false True False");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 4);
    QVERIFY(results[0] == "true");
    QVERIFY(results[1] == "false");
    QVERIFY(results[2] == "true");
    QVERIFY(results[3] == "false");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_Strings()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("\"Hello, World!\" \"NeoAda\" \"Test String\" \"Double\"\"Quotes\"");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 4);
    QVERIFY(results[0] == "Hello, World!");
    QVERIFY(results[1] == "NeoAda");
    QVERIFY(results[2] == "Test String");
    QVERIFY(results[3] == "Double\"Quotes");
}

void TstParser::test_lexer_Range()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("1..10");

    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 3);
    QVERIFY(results[0] == "1");
    QVERIFY(results[1] == "..");
    QVERIFY(results[2] == "10");
}

void TstParser::test_lexer_Comments() {
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript(R"(
        declare x: Natural := 42; -- A comment
        -- Another comment
        declare y: Natural := x + 1;
    )");

    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 16);
    QVERIFY(results[0] == "declare");
    QVERIFY(results[1] == "x");
    QVERIFY(results[2] == ":");
    QVERIFY(results[3] == "Natural");
    QVERIFY(results[4] == ":=");
    QVERIFY(results[5] == "42");
    QVERIFY(results[6] == ";");
    QVERIFY(results[7] == "declare");
    QVERIFY(results[8] == "y");
    QVERIFY(results[9] == ":");
    QVERIFY(results[10] == "Natural");
    QVERIFY(results[11] == ":=");
    QVERIFY(results[12] == "x");
    QVERIFY(results[13] == "+");
    QVERIFY(results[14] == "1");
    QVERIFY(results[15] == ";");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_Expression1()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("(42)");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 3);
    QVERIFY(results[0] == "(");
    QVERIFY(results[1] == "42");
    QVERIFY(results[2] == ")");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_Expression2()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("x := (42);");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 6);
    QVERIFY(results[0] == "x");
    QVERIFY(results[1] == ":=");
    QVERIFY(results[2] == "(");
    QVERIFY(results[3] == "42");
    QVERIFY(results[4] == ")");
    QVERIFY(results[5] == ";");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_HelloWorld()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("print(\"Hello World\");");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 5);
    QVERIFY(results[0] == "print");
    QVERIFY(results[1] == "(");
    QVERIFY(results[2] == "Hello World");
    QVERIFY(results[3] == ")");
    QVERIFY(results[4] == ";");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_HelloWorld2()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("print(\"Hello World\");print(\"Hello World\");");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 10);
    QVERIFY(results[0] == "print");
    QVERIFY(results[1] == "(");
    QVERIFY(results[2] == "Hello World");
    QVERIFY(results[3] == ")");
    QVERIFY(results[4] == ";");
    QVERIFY(results[5] == "print");
    QVERIFY(results[6] == "(");
    QVERIFY(results[7] == "Hello World");
    QVERIFY(results[8] == ")");
    QVERIFY(results[9] == ";");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Declaration1()
{
    std::string script = R"(
        declare x : Number;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Declaration, "x")
    Node(Identifier, "Number")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Declaration2()
{
    std::string script = R"(
        declare x : Number := 42;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Declaration, "x")
    Node(Identifier, "Number")
    Node(Number, "42")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Declaration3()
{
    std::string script = R"(
        volatile x : Number := 42;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Volatile, "x")
    Node(Identifier, "Number")
    Node(Number, "42")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Declaration4_List()
{
    std::string script = R"(
        declare x : List;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Declaration, "x")
    Node(Identifier, "List")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Declaration5_List_Init()
{
    std::string script = R"(
        declare x : List := [1,2,3];
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Declaration, "x")
    Node(Identifier, "List")
    Node(ListLiteral, "")
      Node(Number, "1")
      Node(Number, "2")
      Node(Number, "3")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Factor()
{
    std::string script = R"(
        x := a**b;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "**")
      Node(Identifier, "a")
      Node(Identifier, "b")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Primary1()
{
    std::string script = R"(
        x := a();
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(FunctionCall, "a")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Primary2()
{
    std::string script = R"(
        x := (a());
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(Expression, "")
      Node(FunctionCall, "a")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Relation1()
{
    std::string script = R"(
        x := a and b;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "and")
      Node(Identifier, "a")
      Node(Identifier, "b")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Relation2()
{
    std::string script = R"(
        x := a and b and c;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "and")
      Node(BinaryOperator, "and")
        Node(Identifier, "a")
        Node(Identifier, "b")
      Node(Identifier, "c")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Relation3()
{
    std::string script = R"(
        x := a and b or c xor d;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "xor")
      Node(BinaryOperator, "or")
        Node(BinaryOperator, "and")
          Node(Identifier, "a")
          Node(Identifier, "b")
        Node(Identifier, "c")
      Node(Identifier, "d")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_SimpleExpression1()
{
    std::string script = R"(
        x := -a;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(UnaryOperator, "-")
      Node(Identifier, "a")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_SimpleExpression2()
{
    std::string script = R"(
        x := a > b;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, ">")
      Node(Identifier, "a")
      Node(Identifier, "b")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_SimpleExpression3()
{
    std::string script = R"(
        x := a() > b;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, ">")
      Node(FunctionCall, "a")
      Node(Identifier, "b")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);

}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_SimpleExpression4()
{
    std::string script = R"(
        x := a(z) + 1;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "+")
      Node(FunctionCall, "a")
        Node(Identifier, "z")
      Node(Number, "1")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_SimpleExpression5()
{
    std::string script = R"(
        x := -1 * -2;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(UnaryOperator, "-")
      Node(BinaryOperator, "*")
        Node(Number, "1")
        Node(UnaryOperator, "-")
          Node(Number, "2")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_SimpleExpression6()
{
    std::string script = R"(
        x := x - 1;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "-")
      Node(Identifier, "x")
      Node(Number, "1")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Expression1()
{
    std::string script = R"(
        x := 42;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(Number, "42")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Expression2()
{
    std::string script = R"(
        x := (42);
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(Expression, "")
      Node(Number, "42")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

void TstParser::test_parser_SimpleProgram()
{
    std::string script = R"(
        declare x: Natural;
        x := 42;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Declaration, "x")
    Node(Identifier, "Natural")
  Node(Assignment, "x")
    Node(Number, "42")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

void TstParser::test_parser_SimpleExpression()
{
    std::string script = R"(
        x := 42 + 23;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "+")
      Node(Number, "42")
      Node(Number, "23")
)";
    std::string currentAST =  ast->serialize();

    // std::cout << currentAST;
    // std::cout << expectedAST;

    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Expression3()
{
    std::string script = R"(
        x := 42 + 23;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "+")
      Node(Number, "42")
      Node(Number, "23")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Expression4()
{
    std::string script = R"(
        x := 1 + 2 - 3;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "-")
      Node(BinaryOperator, "+")
        Node(Number, "1")
        Node(Number, "2")
      Node(Number, "3")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Expression5()
{
    std::string script = R"(
        x := 1 + 2 - 3 + 4;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "+")
      Node(BinaryOperator, "-")
        Node(BinaryOperator, "+")
          Node(Number, "1")
          Node(Number, "2")
        Node(Number, "3")
      Node(Number, "4")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Expression6()
{
    std::string script = R"(
        x := a**b + 7;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "+")
      Node(BinaryOperator, "**")
        Node(Identifier, "a")
        Node(Identifier, "b")
      Node(Number, "7")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Expression7()
{
    std::string script = R"(
        x := 7 + a**b;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "+")
      Node(Number, "7")
      Node(BinaryOperator, "**")
        Node(Identifier, "a")
        Node(Identifier, "b")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Expression8()
{
    std::string script = R"(
        x := test();
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(FunctionCall, "test")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Expression9()
{
    std::string script = R"(
        x := 42 + (y * foo(z)) - 5 ** 2;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, "x")
    Node(BinaryOperator, "-")
      Node(BinaryOperator, "+")
        Node(Number, "42")
        Node(Expression, "")
          Node(BinaryOperator, "*")
            Node(Identifier, "y")
            Node(FunctionCall, "foo")
              Node(Identifier, "z")
      Node(BinaryOperator, "**")
        Node(Number, "5")
        Node(Number, "2")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);

}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_HelloWorld()
{
    std::string script = R"(
        print("Hello World");
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(FunctionCall, "print")
    Node(Literal, "Hello World")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_HelloWorld2()
{
    // ";" expected-Bug
    std::string script = R"(
        print("Hello World");
        print("Hello World");
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(FunctionCall, "print")
    Node(Literal, "Hello World")
  Node(FunctionCall, "print")
    Node(Literal, "Hello World")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_FunctionCall1()
{
    std::string script = R"(
        print();
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(FunctionCall, "print")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_MethodCall1()
{
    std::string script = R"(
        string:print(x);
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(StaticMethodCall, "print")
    Node(MethodContext, "string")
    Node(Identifier, "x")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_MethodCall2()
{
    std::string script = R"(
        x.print();
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(InstanceMethodCall, "print")
    Node(MethodContext, "x")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_WhileLoop()
{
    std::string script = R"(

while x < 10 loop
    x := x + 1;
end loop;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(WhileLoop, "")
    Node(BinaryOperator, "<")
      Node(Identifier, "x")
      Node(Number, "10")
    Node(Block, "")
      Node(Assignment, "x")
        Node(BinaryOperator, "+")
          Node(Identifier, "x")
          Node(Number, "1")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_WhileLoopBreak1()
{
    std::string script = R"(

while x < 10 loop
    break;
end loop;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(WhileLoop, "")
    Node(BinaryOperator, "<")
      Node(Identifier, "x")
      Node(Number, "10")
    Node(Block, "")
      Node(Break, "")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_WhileLoopBreak2()
{
    std::string script = R"(

while x < 10 loop
    break when x < 2;
end loop;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(WhileLoop, "")
    Node(BinaryOperator, "<")
      Node(Identifier, "x")
      Node(Number, "10")
    Node(Block, "")
      Node(Break, "")
        Node(BinaryOperator, "<")
          Node(Identifier, "x")
          Node(Number, "2")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_ForLoopRange()
{
    std::string script = R"(

for x in 1..10 loop
    print(x);
end loop;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(ForLoop, "x")
    Node(Range, "")
      Node(Number, "1")
      Node(Number, "10")
    Node(Block, "")
      Node(FunctionCall, "print")
        Node(Identifier, "x")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_If()
{
    std::string script = R"(

if x > 10 then
    print("x > 10");
end if;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(If, "")
    Node(BinaryOperator, ">")
      Node(Identifier, "x")
      Node(Number, "10")
    Node(Block, "")
      Node(FunctionCall, "print")
        Node(Literal, "x > 10")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_IfElse()
{
    std::string script = R"(

if x > 10 then
    print("x > 10");
else
    print("x <= 10");
end if;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(If, "")
    Node(BinaryOperator, ">")
      Node(Identifier, "x")
      Node(Number, "10")
    Node(Block, "")
      Node(FunctionCall, "print")
        Node(Literal, "x > 10")
    Node(Else, "")
      Node(Block, "")
        Node(FunctionCall, "print")
          Node(Literal, "x <= 10")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);

}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_ElseIf1()
{
    std::string script = R"(

if x > 10 then
    print("x > 10");
elsif x > 5 then
    print("x > 5");
else
    print("x <= 5");
end if;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(If, "")
    Node(BinaryOperator, ">")
      Node(Identifier, "x")
      Node(Number, "10")
    Node(Block, "")
      Node(FunctionCall, "print")
        Node(Literal, "x > 10")
    Node(ElseIf, "")
      Node(BinaryOperator, ">")
        Node(Identifier, "x")
        Node(Number, "5")
      Node(Block, "")
        Node(FunctionCall, "print")
          Node(Literal, "x > 5")
    Node(Else, "")
      Node(Block, "")
        Node(FunctionCall, "print")
          Node(Literal, "x <= 5")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_ElseIf2()
{
    std::string script = R"(

if    (x > 10) then   print("x > 10");
elsif (x > 5)  then   print("x > 5");
else                  print("x <= 5");
end if;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(If, "")
    Node(Expression, "")
      Node(BinaryOperator, ">")
        Node(Identifier, "x")
        Node(Number, "10")
    Node(Block, "")
      Node(FunctionCall, "print")
        Node(Literal, "x > 10")
    Node(ElseIf, "")
      Node(Expression, "")
        Node(BinaryOperator, ">")
          Node(Identifier, "x")
          Node(Number, "5")
      Node(Block, "")
        Node(FunctionCall, "print")
          Node(Literal, "x > 5")
    Node(Else, "")
      Node(Block, "")
        Node(FunctionCall, "print")
          Node(Literal, "x <= 5")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_ElseIf3()
{
    std::string script = R"(

if x > 10 then
    print("x > 10");
elsif x > 5 then
    print("x > 5");
elsif x > 0 then
    print("x > 0");
else
    print("x <= 0");
end if;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(If, "")
    Node(BinaryOperator, ">")
      Node(Identifier, "x")
      Node(Number, "10")
    Node(Block, "")
      Node(FunctionCall, "print")
        Node(Literal, "x > 10")
    Node(ElseIf, "")
      Node(BinaryOperator, ">")
        Node(Identifier, "x")
        Node(Number, "5")
      Node(Block, "")
        Node(FunctionCall, "print")
          Node(Literal, "x > 5")
    Node(ElseIf, "")
      Node(BinaryOperator, ">")
        Node(Identifier, "x")
        Node(Number, "0")
      Node(Block, "")
        Node(FunctionCall, "print")
          Node(Literal, "x > 0")
    Node(Else, "")
      Node(Block, "")
        Node(FunctionCall, "print")
          Node(Literal, "x <= 0")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_return()
{
    std::string script = R"(
        return 42;
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Return, "")
    Node(Number, "42")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Procedure1()
{
    std::string script = R"(

procedure HelloWorld is
begin
    print("Hello, World!");
end;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Procedure, "HelloWorld")
    Node(Parameters, "")
    Node(Block, "")
      Node(FunctionCall, "print")
        Node(Literal, "Hello, World!")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Procedure2()
{
    std::string script = R"(

procedure HelloWorld(msg : any) is
begin
    print(msg);
end;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Procedure, "HelloWorld")
    Node(Parameters, "")
      Node(Parameter, "msg")
        Node(Identifier, "any")
    Node(Block, "")
      Node(FunctionCall, "print")
        Node(Identifier, "msg")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Function1()
{
    std::string script = R"(

function Add(a : Natural; b : Natural) return Natural is
begin
    return a + b;
end;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Function, "Add")
    Node(Parameters, "")
      Node(Parameter, "a")
        Node(Identifier, "Natural")
      Node(Parameter, "b")
        Node(Identifier, "Natural")
    Node(ReturnType, "Natural")
    Node(Block, "")
      Node(Return, "")
        Node(BinaryOperator, "+")
          Node(Identifier, "a")
          Node(Identifier, "b")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Method1()
{
    std::string script = R"(

function string:length() return Natural is
begin
end;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Function, "length")
    Node(MethodContext, "string")
    Node(Parameters, "")
    Node(ReturnType, "Natural")
    Node(Block, "")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Method2()
{
    std::string script = R"(

procedure string:trim() is
begin
end;

    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Procedure, "trim")
    Node(MethodContext, "string")
    Node(Parameters, "")
    Node(Block, "")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_state_Declarations()
{
    NadaState state;

    // enum Type { Undefined, Any, Number, Natural, Supernatural, Boolean, Byte, Character, String, Struct };
    QCOMPARE(state.define("a", "Any"), true);
    QCOMPARE(state.define("b", "Number"), true);
    QCOMPARE(state.define("c", "Natural"), true);
    QCOMPARE(state.define("d", "Supernatural"), true);
    QCOMPARE(state.define("e", "Boolean"), true);
    QCOMPARE(state.define("f", "Byte"), true);
    QCOMPARE(state.define("g", "Character"), true);
    QCOMPARE(state.define("h", "String"), true);
    QCOMPARE(state.define("i", "Struct"), true);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_state_GlobalScope()
{
    NadaState state;

    // level "0" variable;
    QCOMPARE(state.define("a", "Number"), true);
    QCOMPARE(state.typeOf("a"), Nda::Number);

    // level "1" variable;
    state.pushScope(NadaSymbolTable::ConditionalScope); // enter "if"
    QCOMPARE(state.define("a", "String"), true);
    QCOMPARE(state.typeOf("a"), Nda::String);

    // level "2" variable
    state.pushScope(NadaSymbolTable::ConditionalScope); // enter "if"
    QCOMPARE(state.typeOf("a"), Nda::String);    // found level 1 "a"
    QCOMPARE(state.define("a", "Natural"), true);
    QCOMPARE(state.typeOf("a"), Nda::Natural);

    state.popScope(); // leave "if"
    QCOMPARE(state.typeOf("a"), Nda::String);

    state.popScope(); // leave "if"
    QCOMPARE(state.typeOf("a"), Nda::Number);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Declarations()
{
    //  enum Type { Undefined, Any, Number, Natural, Supernatural, Boolean, Byte, Character, String, Struct };

    std::string script = R"(
        declare x: Natural;

        declare a: Any;
        declare b: Number;
        declare c: Supernatural;
        declare d: Boolean;
        declare e: Byte;
        declare f: Character;
        declare g: String;
        declare h: Struct;

    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QCOMPARE(state.typeOf("x"), Nda::Natural);
    QCOMPARE(state.typeOf("z"), Nda::Undefined);

    QCOMPARE(state.typeOf("a"), Nda::Any);
    QCOMPARE(state.typeOf("b"), Nda::Number);
    QCOMPARE(state.typeOf("c"), Nda::Supernatural);
    QCOMPARE(state.typeOf("d"), Nda::Boolean);
    QCOMPARE(state.typeOf("e"), Nda::Byte);
    QCOMPARE(state.typeOf("f"), Nda::Character);
    QCOMPARE(state.typeOf("g"), Nda::String);
    QCOMPARE(state.typeOf("h"), Nda::Struct);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ProcedureCall()
{
    std::string script = R"(
        print("hello NeoAda");
        print(42);
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bind("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> NadaValue {
        results.push_back(args.at("message").toString());
        return NadaValue();
    });

    interpreter.execute(ast);


    QVERIFY(results.size() == 2);
    QVERIFY(results[0] == "hello NeoAda");
    QVERIFY(results[1] == "42");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ifStatement()
{
    std::string script = R"(
        declare x : Number := 42.0;
        if x > 5 then
            print("x>5");
        end if;
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bind("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> NadaValue {
        results.push_back(args.at("message").toString());
        return NadaValue();
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 1);
    QVERIFY(results[0] == "x>5");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ifElseStatement()
{
    std::string script = R"(
        declare x : Number := 4.0;
        if x > 5 then
            print("x>5");
        else
            print("x<=5");
        end if;
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bind("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> NadaValue {
        results.push_back(args.at("message").toString());
        return NadaValue();
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 1);
    QVERIFY(results[0] == "x<=5");

}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_whileStatement()
{
    std::string script = R"(
        declare x : Natural := 2;
        while x > 0 loop
            print(x);
            x := x - 1;
        end loop;
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bind("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> NadaValue {
        results.push_back(args.at("message").toString());
        return NadaValue();
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 2);
    QVERIFY(results[0] == "2");
    QVERIFY(results[1] == "1");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_whileBreak()
{
    std::string script = R"(
        declare x : Natural := 10;
        while x > 0 loop
            print(x);
            x := x - 1;
            if x < 9 then
                break;
            end if;
        end loop;
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bind("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> NadaValue {
        results.push_back(args.at("message").toString());
        return NadaValue();
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 2);
    QVERIFY(results[0] == "10");
    QVERIFY(results[1] == "9");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_whileBreakWhen()
{
    std::string script = R"(
        declare x : Natural := 10;
        while x > 0 loop
            print(x);
            x := x - 1;
            break when x < 9;
        end loop;
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bind("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> NadaValue {
        results.push_back(args.at("message").toString());
        return NadaValue();
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 2);
    QVERIFY(results[0] == "10");
    QVERIFY(results[1] == "9");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_whileContinue()
{
    std::string script = R"(
        declare x : Natural := 10;
        while x > 0 loop
            x := x - 1;

            if x > 0 then
                continue;
            end if;
            print(x);
        end loop;
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bind("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> NadaValue {
        results.push_back(args.at("message").toString());
        return NadaValue();
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 1);
    QVERIFY(results[0] == "0");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Return1()
{
    std::string script = R"(
        return 42;
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "42");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Return2()
{
    std::string script = R"(
        return true;
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toBool(nullptr) == true);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Return3()
{
    std::string script = R"(
        return false;
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toBool(nullptr) == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Volatile_CTor()
{
    std::string script = R"(
        volatile x : Natural;
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.onVolatileCtor([&](const std::string &name,  NadaValue& val) -> void {
        results.push_back(name);
        results.push_back(val.type() == Nda::Natural ? "Natural" : "?");
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 2);
    QVERIFY(results[0] == "x");
    QVERIFY(results[1] == "Natural");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_static_method()
{
    std::string script = R"(
        return string:number(10);
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;

    state.bind("string","number",{{"n", "natural", Nda::InMode}}, [&](const Nda::FncValues& args) -> NadaValue {
        std::string       s;
        std::stringstream ss(s);
        ss << args.at("n").toInt64();

        NadaValue ret;
        ret.fromString(s);
        return ret;
    });


    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "10");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Literals()
{
    QVERIFY(NeoAda::evaluate("return true;").toBool() == true);
    QVERIFY(NeoAda::evaluate("return false;").toBool() == false);
    QVERIFY(NeoAda::evaluate("return 42;").toInt64() == 42);
    QVERIFY(NeoAda::evaluate("return -1;").toInt64() == -1);

    QVERIFY(NeoAda::evaluate("return  10_000;").toInt64() == +10000);
    QVERIFY(NeoAda::evaluate("return -10_000;").toInt64() == -10000);
    QVERIFY(NeoAda::evaluate("return +10_000;").toInt64() == +10000);

    QVERIFY(NeoAda::evaluate("return  2#1001_1000#;").toInt64() ==  152);

    QVERIFY(NeoAda::evaluate("return \"NeoAda\";").toString() == "NeoAda");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Length()
{
    QVERIFY(NeoAda::evaluate("return  #1;").toInt64()   == 1);
    QVERIFY(NeoAda::evaluate("return  #1.1;").toInt64() == 1);
    QVERIFY(NeoAda::evaluate("return #\"\";").toInt64() == 0);
    QVERIFY(NeoAda::evaluate("return #\"NeoAda\";").toInt64() == 6);

    QVERIFY(NeoAda::evaluate("return #[];").toInt64()      == 0);
    QVERIFY(NeoAda::evaluate("return #[1];").toInt64()     == 1);
    QVERIFY(NeoAda::evaluate("return #[1,2,3];").toInt64() == 3);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Equal()
{
    QVERIFY(NeoAda::evaluate("return true = true;").toBool()  == true);
    QVERIFY(NeoAda::evaluate("return true = false;").toBool() == false);
    QVERIFY(NeoAda::evaluate("return 42 = 42;").toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42.5 = 42.5;").toBool()  == true);
    QVERIFY(NeoAda::evaluate("return 42 = 23;").toBool()      == false);

    QVERIFY(NeoAda::evaluate("return \"NeoAda\" = \"NeoAda\";").toBool() == true);
    QVERIFY(NeoAda::evaluate("return \"NeoAda\" = \"neoada\";").toBool() == false); // case insensitive as in Ada95


    QVERIFY(NeoAda::evaluate("return (true = true);" ).toBool() == true);
    QVERIFY(NeoAda::evaluate("return (true = false);").toBool() == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_NotEqual()
{
    QVERIFY(NeoAda::evaluate("return true <> true;").toBool()  == false);
    QVERIFY(NeoAda::evaluate("return true <> false;").toBool() == true);
    QVERIFY(NeoAda::evaluate("return 42   <> 42;").toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 42.5 <> 42.5;").toBool()  == false);
    QVERIFY(NeoAda::evaluate("return 42   <> 23;").toBool()      == true);

    QVERIFY(NeoAda::evaluate("return \"NeoAda\" <> \"NeoAda\";").toBool() == false);
    QVERIFY(NeoAda::evaluate("return \"NeoAda\" <> \"neoada\";").toBool() == true); // case insensitive as in Ada95
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_LessThan()
{
    // Natural
    QVERIFY(NeoAda::evaluate("return 42 < 42;").toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23 < 42;").toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42 < 23;").toBool()      == false);

    // Number
    QVERIFY(NeoAda::evaluate("return 42.0 < 42.0;").toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23.0 < 42.0;").toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42.0 < 23.0;").toBool()      == false);

}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_LessEqualThan()
{
    // Natural
    QVERIFY(NeoAda::evaluate("return 42 <= 42;").toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 23 <= 42;").toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42 <= 23;").toBool()      == false);

    // Number
    QVERIFY(NeoAda::evaluate("return 42.0 <= 42.0;").toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 23.0 <= 42.0;").toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42.0 <= 23.0;").toBool()      == false);

}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_ConcatString()
{
    QVERIFY(NeoAda::evaluate("return \"Neo\" & \"Ada\";").toString()            == "NeoAda");
    QVERIFY(NeoAda::evaluate("return \"Neo\" & \" \" & \"Ada\";").toString()    == "Neo Ada");
    QVERIFY(NeoAda::evaluate("return \"Neo\" & \"Ada\" = \"NeoAda\";").toBool() == true);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Modulo()
{
    QVERIFY(NeoAda::evaluate("return 42 mod 21;").toInt64() == 0);
    QVERIFY(NeoAda::evaluate("return 42 mod 20;").toInt64() == 2);
    QVERIFY(NeoAda::evaluate("return  5 mod 6;").toInt64()  == 5);
    QVERIFY(NeoAda::evaluate("return  4 mod 4;").toInt64()  == 0);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Multiply()
{
    QVERIFY(NeoAda::evaluate("return 2   * 21;").toInt64()   == 42);
    QVERIFY(NeoAda::evaluate("return 2.0 * 21.0;").toInt64() == 42);
    QVERIFY(NeoAda::evaluate("return -42 * -1;").toInt64()   == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Relations()
{
    // AND
    QVERIFY(NeoAda::evaluate("return true  and true;").toBool()   == true);
    QVERIFY(NeoAda::evaluate("return false and true;").toBool()   == false);
    QVERIFY(NeoAda::evaluate("return true  and false;").toBool()  == false);
    QVERIFY(NeoAda::evaluate("return false and false;").toBool()  == false);

    // OR
    QVERIFY(NeoAda::evaluate("return true  or true;").toBool()   == true);
    QVERIFY(NeoAda::evaluate("return false or true;").toBool()   == true);
    QVERIFY(NeoAda::evaluate("return true  or false;").toBool()  == true);
    QVERIFY(NeoAda::evaluate("return false or false;").toBool()  == false);

    // XOR
    QVERIFY(NeoAda::evaluate("return true  xor true;").toBool()   == false);
    QVERIFY(NeoAda::evaluate("return false xor true;").toBool()   == true);
    QVERIFY(NeoAda::evaluate("return true  xor false;").toBool()  == true);
    QVERIFY(NeoAda::evaluate("return false xor false;").toBool()  == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_List_Init()
{
    std::string script = R"(
        declare x : List := [1,2,3];
        print(x);
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bind("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> NadaValue {
        results.push_back(args.at("message").toString());
        return NadaValue();
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 1);
    QVERIFY(results[0] == "[1,2,3]");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_GlobalValue()
{
    std::string script = R"(

    declare x : Natural := 42;

    if x > 10 then
        -- x is global scope
        if x > 40 then
            return true;
        end if;
    end if;

    return false;
    )";

    QVERIFY(NeoAda::evaluate(script).toBool() == true);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_ScopeValue()
{
    std::string script = R"(

    declare x : Natural := 42;

    if x > 10 then
        declare x : Natural := 10;
        -- x is local scope... not nice.. but legal..
        if x > 40 then
            return true;
        end if;
    end if;

    return false;
    )";

    QVERIFY(NeoAda::evaluate(script).toBool() == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_WhileBreak()
{
    std::string script = R"(

    declare x : Natural := 3;

    while x > 0 loop
        if x = 2 then
            break;
        end if;
        x := x - 1;
    end loop;

    if x = 2 then
        return true;
    end if;

    return false;
    )";

    QVERIFY(NeoAda::evaluate(script).toBool() == true);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_WhileContinue()
{
    std::string script = R"(

    declare x : Natural := 3;
    declare y : Natural := 3;

    while x > 0 loop
        if x > 1 then
            x := x - 1;
            continue;
        end if;
        x := x - 1;
        y := y - 1;
    end loop;

    if x = 0 and y = 2 then
        return true;
    end if;

    return false;
    )";

    QVERIFY(NeoAda::evaluate(script).toBool() == true);

}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_ForLoop()
{
    std::string script = R"(

    declare x : Natural := 0;

    for i in 1..10 loop
        x := x + 1;
    end loop;

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toInt64() == 10);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_ForLoopBreak()
{
    std::string script = R"(

    declare x : Natural := 0;

    for i in 1..10 loop
        x := x + 1;
        break when i > 5;
    end loop;

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toInt64() == 6);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_ForLoopContinue()
{
    std::string script = R"(

    declare x : Natural := 0;

    for i in 1..10 loop
        continue when i > 5;
        x := x + 1;
    end loop;

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toInt64() == 5);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Procedure1()
{
    std::string script = R"(

    declare x : Natural := 0;

    procedure HelloWorld is
    begin
        x := x + 42;
    end;

    HelloWorld();
    HelloWorld();

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toInt64() == 2*42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Procedure2()
{
    std::string script = R"(

    declare x : Natural := 1;

    procedure multiply(factor : any) is
    begin
        x := x * factor;
    end;

    multiply(42);

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Procedure3_Return()
{
    std::string script = R"(

    declare x : Natural := 1;

    procedure multiply(factor : any) is
    begin
        if factor > 50 then
            return;
        end if;
        x := x * factor;
    end;

    multiply(42);
    multiply(52);

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Procedure4_Out()
{
    std::string script = R"(

    declare x : String := "Hello";

    procedure Hello(s : out string) is
    begin
        s := s & ", World";
    end;

    Hello("NeoAda"); -- no effect -> warning?
    Hello(x);

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toString() == "Hello, World");
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Function1()
{
    std::string script = R"(

    declare x : Natural;

    function Add(a : Natural; b : Natural) return Natural is
    begin
        return a + b;
    end;

    x := Add(40, 2);

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Function2()
{
    std::string script = R"(

    function CreateHelloWorld() return any is
    begin
        return "Hello, World";
    end;

    declare x : any := CreateHelloWorld();

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toString() == "Hello, World");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Function2_Uppercase()
{
    std::string script = R"(

    FUNCTION CreateHelloWorld() RETURN Any IS
    BEGIN
        RETURN "Hello, World";
    END;

    DECLARE x : Any := CreateHelloWorld();

    RETURN x;
    )";

    QVERIFY(NeoAda::evaluate(script).toString() == "Hello, World");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Static_Method1()
{
    std::string script = R"(

    declare x : string;

    procedure string:sayHello() is
    begin
        x := "Hello, World";
    end;

    string:sayHello();

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toString() == "Hello, World");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Static_Method2()
{
    std::string script = R"(

    function string:hello() return string is
    begin
        return "Hello, World";
    end;

    declare x : any := string:hello();

    return x;
    )";

    QVERIFY(NeoAda::evaluate(script).toString() == "Hello, World");
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Instance_Method1()
{
    std::string script = R"(

    function string:hello() return string is
    begin
        return this & ", World";
    end;

    declare x : string := "Hello";

    return x.hello();
    )";

    QVERIFY(NeoAda::evaluate(script).toString() == "Hello, World");
}

//-------------------------------------------------------------------------------------------------
//                                       ERROR HANDLING
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_lexer_invalidCharacter()
{
    NadaLexer lexer;
    NadaException ex;

    lexer.setScript("$");
    try {
        while (lexer.nextToken());
    } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidCharacter);
    QVERIFY(ex.line()   == 1);
    QVERIFY(ex.column() == 1);

    lexer.setScript("declare \n $");
    try {
        while (lexer.nextToken());
    } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidCharacter);
    QVERIFY(ex.line()   == 2);
    QVERIFY(ex.column() == 2);
    // std::cout << ex.what() << std::endl;
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_lexer_invalidString()
{
    NadaLexer lexer;
    NadaException ex;

    lexer.setScript("\"23456789");
    try {
        while (lexer.nextToken());
    } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);
    QVERIFY(ex.line()   == 1);
    QVERIFY(ex.column() == 9);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_lexer_invalidBasedLiteral()
{
    NadaLexer lexer;
    NadaException ex;

    //               2#1000_0100#
    lexer.setScript("2#1000_0100 ");
    try {
        while (lexer.nextToken());
    } catch (NadaException &e) {
        ex = e;
    }
    // std::cout << ex.what() << std::endl;
    QVERIFY(ex.code()   == Nada::Error::InvalidBasedLiteral);
    QVERIFY(ex.line()   == 1);
    QVERIFY(ex.column() == 12);

    lexer.setScript("5%5 ");
    try {
        while (lexer.nextToken());
    } catch (NadaException &e) {
        ex = e;
    }
    // std::cout << ex.what() << std::endl;
    QVERIFY(ex.code()   == Nada::Error::InvalidCharacter);

    lexer.setScript("16#FFG ");
    try {
        while (lexer.nextToken());
    } catch (NadaException &e) {
        ex = e;
    }
    // std::cout << ex.what() << std::endl;
    QVERIFY(ex.code()   == Nada::Error::InvalidBasedLiteral);

}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_parser_declaration1()
{
    std::string script = R"(
        declare 23
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    NadaException ex;

    try {
        parser.parse(script);
    } catch (NadaException &e) {
        ex = e;
    }

    QVERIFY(ex.code()   == Nada::Error::IdentifierExpected);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_parser_declaration2()
{
    std::string script = R"(
        declare x =
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    NadaException ex;

    try {
        parser.parse(script);
    } catch (NadaException &e) {
        ex = e;
    }

    QVERIFY(ex.code()   == Nada::Error::InvalidToken);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_parser_declaration3()
{
    std::string script = R"(
        declare x : 23
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    NadaException ex;

    try {
        parser.parse(script);
    } catch (NadaException &e) {
        ex = e;
    }

    QVERIFY(ex.code()   == Nada::Error::IdentifierExpected);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_parser_declaration4()
{
    std::string script = R"(
        declare x : Number :=
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    NadaException ex;

    try {
        parser.parse(script);
    } catch (NadaException &e) {
        ex = e;
    }

    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_parser_if1()
{
    std::string script = R"(
        if x
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    NadaException ex;

    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }

    QVERIFY(ex.code()   == Nada::Error::KeywordExpected);

    script = "if x then";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then y";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidToken);

    script = "if x then y()";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then y() end";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidToken);

    script = "if x then y(); end";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then y(); end if";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then y(); end if;";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::NoError);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_parser_ifElse1()
{
    std::string script = R"(
        if x then f(); else
    )";

    NadaLexer lexer;
    NadaParser parser(lexer);
    NadaException ex;

    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then f(); else y";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidToken);

    script = "if x then f(); else y()";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);


    script = "if x then f(); else y() end";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidToken);

    script = "if x then f(); else y(); end";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then f(); else y(); end;";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::KeywordExpected);

    script = "if x then f(); else y(); end while;";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::KeywordExpected);

    script = "if x then f(); else y(); end if";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then f(); else y(); end if;";
    ex = NadaException();
    try { parser.parse(script); } catch (NadaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::NoError);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_interpreter_stringAssignment()
{
    {
        std::string script = R"(

    declare x : string := 42;

    )";

        NeoAda::Exception ex;
        NeoAda::evaluate(script, &ex);
        QVERIFY(ex.code() == Nada::Error::AssignmentError);
    }

    {
        std::string script = R"(

    declare x : string;
    x := 42;

    )";

        NeoAda::Exception ex;
        NeoAda::evaluate(script, &ex);
        QVERIFY(ex.code() == Nada::Error::AssignmentError);
    }

    {
        std::string script = R"(

    declare x : string := true;

    )";

        NeoAda::Exception ex;
        NeoAda::evaluate(script, &ex);
        QVERIFY(ex.code() == Nada::Error::AssignmentError);
    }

    {
        std::string script = R"(

    declare x : string := 1.23;

    )";

        NeoAda::Exception ex;
        NeoAda::evaluate(script, &ex);
        QVERIFY(ex.code() == Nada::Error::AssignmentError);
    }
}

void TstParser::test_error_interpreter_boolAssignment()
{
    {
        std::string script = R"(

    declare x : boolean := 42;

    )";

        NeoAda::Exception ex;
        NeoAda::evaluate(script, &ex);
        QVERIFY(ex.code() == Nada::Error::AssignmentError);
    }

    {
        std::string script = R"(

    declare x : boolean := "true";

    )";

        NeoAda::Exception ex;
        NeoAda::evaluate(script, &ex);
        QVERIFY(ex.code() == Nada::Error::AssignmentError);
    }
}


QTEST_APPLESS_MAIN(TstParser)

#include "tst_parser.moc"

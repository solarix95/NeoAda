#include <QtTest>

#include <iostream>

#include <libneoada/lexer.h>
#include <libneoada/parser.h>
#include <libneoada/state.h>
#include <libneoada/interpreter.h>



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

    // Lexer Literals
    void test_lexer_Numbers();
    void test_lexer_OperatorsAndSymbols();
    void test_lexer_Identifiers();
    void test_lexer_Strings();
    void test_lexer_Comments();
    void test_lexer_Expression1();
    void test_lexer_Expression2();
    void test_lexer_HelloWorld();


    void test_parser_Declaration1();
    void test_parser_Declaration2();

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

    void test_parser_SimpleProgram();
    void test_parser_SimpleExpression();

    void test_parser_FunctionCall1();

    void test_state_Declarations();

    void test_interpreter_Declarations();
    void test_interpreter_ProcedureCall();
};

TstParser::TstParser() {}

TstParser::~TstParser() {}

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
void TstParser::test_lexer_Strings()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("\"Hello, World!\" \"NeoAda\" \"Test String\" \"Double\"\"Quotes\" \"1\"\"2\"");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 6);
    QVERIFY(results[0] == "\"Hello, World!\"");
    QVERIFY(results[1] == "\"NeoAda\"");
    QVERIFY(results[2] == "\"Test String\"");
    QVERIFY(results[3] == "\"Double\"\"Quotes\"");

    // Useccase "1""2"
    QVERIFY(results[4] == "\"1\"");
    QVERIFY(results[5] == "\"2\"");
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
    QVERIFY(results[2] == "\"Hello World\"");
    QVERIFY(results[3] == ")");
    QVERIFY(results[4] == ";");
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
    Node(Literal, ""Hello World"")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_FunctionCall1()
{
    std::string script = R"(
        print()
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
void TstParser::test_state_Declarations()
{
    NadaState state;

    // enum Type { Undefined, Any, Number, Natural, Supernatural, Boolean, Byte, Character, String, Struct };
    QCOMPARE(state.declareGlobal("a", "Any"), true);
    QCOMPARE(state.declareGlobal("b", "Number"), true);
    QCOMPARE(state.declareGlobal("c", "Natural"), true);
    QCOMPARE(state.declareGlobal("d", "Supernatural"), true);
    QCOMPARE(state.declareGlobal("e", "Boolean"), true);
    QCOMPARE(state.declareGlobal("f", "Byte"), true);
    QCOMPARE(state.declareGlobal("g", "Character"), true);
    QCOMPARE(state.declareGlobal("h", "String"), true);
    QCOMPARE(state.declareGlobal("i", "Struct"), true);
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

    QCOMPARE(state.typeOfGlobal("x"), Nada::Natural);
    QCOMPARE(state.typeOfGlobal("X"), Nada::Natural);
    QCOMPARE(state.typeOfGlobal("z"), Nada::Undefined);

    QCOMPARE(state.typeOfGlobal("a"), Nada::Any);
    QCOMPARE(state.typeOfGlobal("b"), Nada::Number);
    QCOMPARE(state.typeOfGlobal("c"), Nada::Supernatural);
    QCOMPARE(state.typeOfGlobal("d"), Nada::Boolean);
    QCOMPARE(state.typeOfGlobal("e"), Nada::Byte);
    QCOMPARE(state.typeOfGlobal("f"), Nada::Character);
    QCOMPARE(state.typeOfGlobal("g"), Nada::String);
    QCOMPARE(state.typeOfGlobal("h"), Nada::Struct);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ProcedureCall()
{
    std::string script = R"(
        print("hello NeoAda");
    )";

    NadaLexer       lexer;
    NadaParser      parser(lexer);
    NadaState       state;
    NadaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    state.bind("print",{{"message", "String"}}, [](const NadaFncParameters& args) -> NadaValue {

        // std::cout << args["message"].toString() << std::endl;
        std::cout << "GUGUSLI" << std::endl;

        return NadaValue();
    });

    auto ret = interpreter.execute(ast);
}

QTEST_APPLESS_MAIN(TstParser)

#include "tst_parser.moc"

#include <QtTest>
#include <libneoada/lexer.h>
#include <iostream>

// add necessary includes here

class Parser : public QObject
{
    Q_OBJECT

public:
    Parser();
    ~Parser();

private slots:

    // Lexer Literals
    void test_lexer_Numbers();
    void test_lexer_OperatorsAndSymbols();
    void test_lexer_Identifiers();
    void test_lexer_Strings();
    void test_lexer_Comments();
};

Parser::Parser() {}

Parser::~Parser() {}

/*-----------------------------------------------------------------------------------------------*\
                                       LEXER LITERALS
\*-----------------------------------------------------------------------------------------------*/


//-------------------------------------------------------------------------------------------------
void Parser::test_lexer_Numbers()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.parse("123 1_000.0 42E+3 16#FF# 2#1010#E+2", [&](const std::string& token, NadaLexer::TokenType) {
        results.push_back(token);
    });

    QVERIFY(results.size() == 5);
    QVERIFY(results[0] == "123");
    QVERIFY(results[1] == "1_000.0");
    QVERIFY(results[2] == "42E+3");
    QVERIFY(results[3] == "16#FF#");
    QVERIFY(results[4] == "2#1010#E+2");
}

//-------------------------------------------------------------------------------------------------
void Parser::test_lexer_OperatorsAndSymbols()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.parse(":= ** <= >= + - * / > < ( ) ;", [&](const std::string& token, NadaLexer::TokenType) {
        results.push_back(token);
    });

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
void Parser::test_lexer_Identifiers()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.parse("myVar _private x123", [&](const std::string& token, NadaLexer::TokenType) {
        results.push_back(token);
    });

    QVERIFY(results.size() == 3);
    QVERIFY(results[0] == "myVar");
    QVERIFY(results[1] == "_private");
    QVERIFY(results[2] == "x123");
}

//-------------------------------------------------------------------------------------------------
void Parser::test_lexer_Strings()
{
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.parse("\"Hello, World!\" \"NeoAda\" \"Test String\" \"Double\"\"Quotes\"", [&](const std::string& token, NadaLexer::TokenType) {
        results.push_back(token);
    });

    QVERIFY(results.size() == 4);
    QVERIFY(results[0] == "\"Hello, World!\"");
    QVERIFY(results[1] == "\"NeoAda\"");
    QVERIFY(results[2] == "\"Test String\"");
    QVERIFY(results[3] == "\"Double\"\"Quotes\"");
}

void Parser::test_lexer_Comments() {
    NadaLexer lexer;
    std::vector<std::string> results;

    lexer.parse(R"(
        declare x: Natural := 42; -- A comment
        -- Another comment
        declare y: Natural := x + 1;
    )", [&](const std::string& token, NadaLexer::TokenType) {
                    results.push_back(token);
                });

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
QTEST_APPLESS_MAIN(Parser)

#include "tst_parser.moc"

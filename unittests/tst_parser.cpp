#include "neoadaapi.h"
#include <QtTest>
#include <QDebug>
#include <QCoreApplication>

#include <iostream>
#include <sstream>
#include <fstream>

#include <libneoada/exception.h>
#include <libneoada/lexer.h>
#include <libneoada/parser.h>
#include <libneoada/state.h>
#include <libneoada/interpreter.h>
#include <libneoada/runtime.h>
#include <libneoada/value.h>
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
    void test_core_NumericLiterals();
    void test_core_NumericValues();
    void test_core_NumericValues_Invalid();
    void test_core_VariantToString_Boolean();
    void test_core_VariantToString_Byte();
    void test_core_SharedString();
    void test_core_Assignment();
    void test_core_References();
    void test_core_List_COW();
    void test_core_List_REF();
    void test_core_List_Contains();
    void test_core_List_Concat();

    void test_core_Dict_COW();

    void test_core_Value_CTor();


    // Lexer Literals
    void test_lexer_Numbers();
    void test_lexer_OperatorsAndSymbols();
    void test_lexer_Identifiers();
    void test_lexer_Boolean();
    void test_lexer_Strings();
    void test_lexer_Range();
    void test_lexer_ArrayInit();
    void test_lexer_ArrayAccess();
    void test_lexer_DictInit();
    void test_lexer_DictAccess();
    void test_lexer_Comments();
    void test_lexer_Expression1();
    void test_lexer_Expression2();
    void test_lexer_HelloWorld();
    void test_lexer_HelloWorld2();

    void test_parser_Declaration1();
    void test_parser_Declaration2();
    void test_parser_Declaration3();
    void test_parser_Declaration4_List();
    void test_parser_Declaration5_List_Init1();
    void test_parser_Declaration5_List_Init2();
    void test_parser_Declaration6_Expression();

    void test_parser_Declaration10_Dict();

    void test_parser_With();
    void test_parser_Type();
    void test_parser_Exception();
    void test_parser_ExceptionReraise();

    void test_parser_Factor();
    void test_parser_Primary1();
    void test_parser_Primary2();
    void test_parser_Primary3_List_Access();
    void test_parser_Primary4_ListInList();
    void test_parser_Primary5_DictInDict();

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
    void test_parser_MethodCall3_Chained();

    void test_parser_WhileLoop();
    void test_parser_WhileLoopBreak1();
    void test_parser_WhileLoopBreak2();

    void test_parser_ForLoopRange();

    void test_parser_If();
    void test_parser_IfElse();
    void test_parser_ElseIf1();
    void test_parser_ElseIf2();
    void test_parser_ElseIf3();

    void test_parser_return1();
    void test_parser_return2();

    void test_parser_Procedure1();
    void test_parser_Procedure2();
    void test_parser_Function1();
    void test_parser_Method1();
    void test_parser_Method2();


    void test_state_Declarations();
    void test_state_GlobalScope();

    void test_interpreter_Declarations1();
    void test_interpreter_Declarations2();

    void test_interpreter_WithAddon();
    void test_interpreter_ProcedureCall();
    void test_interpreter_ifStatement();
    void test_interpreter_ifElseStatement();
    void test_interpreter_ifElseIfStatement();
    void test_interpreter_whileStatement();
    void test_interpreter_whileBreak();
    void test_interpreter_whileBreakWhen();
    void test_interpreter_whileContinue();
    void test_interpreter_Return1();
    void test_interpreter_Return2();
    void test_interpreter_Return3();
    void test_interpreter_Return4();
    void test_interpreter_Return5_Natural();
    void test_interpreter_Return_ForLoop();
    void test_interpreter_Return6_Supernatural();
    void test_interpreter_Return7_Number();
    void test_interpreter_Return8_Byte();

    void test_interpreter_ExceptionHandled();
    void test_interpreter_ExceptionUnhandled();
    void test_interpreter_ExceptionCatchAll();
    void test_interpreter_ExceptionConstraintError();
    void test_interpreter_ExceptionProgramError();
    void test_interpreter_ExceptionReraiseSpecific();
    void test_interpreter_ExceptionReraiseAnonymous();

    void test_interpreter_CustomType_TypeIs();
    void test_interpreter_CustomType_Procedure();

    void test_interpreter_Volatile_CTor();
    void test_interpreter_Volatile_Read();
    void test_interpreter_Volatile_ReadFailed();
    void test_interpreter_Volatile_ReadDict();
    void test_interpreter_Volatile_Write();
    void test_interpreter_Volatile_WriteRejected();
    void test_interpreter_Volatile_WriteDict();
    void test_interpreter_Volatile_WriteDictRejected();

    void test_interpreter_static_method();

    void test_api_evaluate_Literals();
    void test_api_evaluate_Length();
    void test_api_evaluate_Equal();
    void test_api_evaluate_NotEqual();
    void test_api_evaluate_LessThan();
    void test_api_evaluate_SignedUnsignedComparisonEdges();
    void test_api_evaluate_LessEqualThan();

    void test_api_evaluate_ConcatString();
    void test_api_evaluate_Modulo();
    void test_api_evaluate_Multiply();
    void test_api_evaluate_Relations();

    void test_api_evaluate_List_Init();
    void test_api_evaluate_List_Read1();
    void test_api_evaluate_List_Read2();
    void test_api_evaluate_List_Write();
    void test_api_evaluate_List_Swap();

    void test_api_evaluate_Bytes_Init();
    void test_api_evaluate_Bytes_RejectMethodWithoutAddon();
    void test_api_evaluate_Bytes_AppendReadWrite();
    void test_api_evaluate_Bytes_Clear();
    void test_api_evaluate_Bytes_AppendBytesContainsIndexOf();
    void test_api_evaluate_Bytes_InsertRemove();
    void test_api_evaluate_Bytes_MidSlicedSlice();
    void test_api_evaluate_Bytes_ChoppedChop();
    void test_api_evaluate_Bytes_RejectNonByteAppend();
    void test_api_evaluate_Bytes_RejectNonByteWrite();
    void test_api_evaluate_Bytes_RejectDictAccessSyntax();

    void test_api_evaluate_Dict_Init();
    void test_api_evaluate_Dict_Append();
    void test_api_evaluate_Dict_Embedded();

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

    // Addition Operator
    void test_api_evaluate_add_Number();
    void test_api_evaluate_add_Natural();
    void test_api_evaluate_add_Supernatural();
    void test_api_evaluate_add_Byte();

    // Subtraction Operator
    void test_api_evaluate_sub_Number();
    void test_api_evaluate_sub_Natural();
    void test_api_evaluate_sub_Supernatural();
    void test_api_evaluate_sub_Byte();

    // Division Operator
    void test_api_evaluate_div_Number();
    void test_api_evaluate_div_Natural();
    void test_api_evaluate_div_Supernatural();
    void test_api_evaluate_div_Byte();

    // Multiply Operator
    void test_api_evaluate_mul_Number();
    void test_api_evaluate_mul_Natural();
    void test_api_evaluate_mul_Supernatural();
    void test_api_evaluate_mul_Byte();

    // Power Operator
    void test_api_evaluate_pow();

    // Modulo Operator
    void test_api_evaluate_mod_Number(); // Modulo on Number is invalide at all..
    void test_api_evaluate_mod_Natural();
    void test_api_evaluate_mod_Supernatural();
    void test_api_evaluate_mod_Byte();

    // runtime LIST addons
    void test_api_runtime_AdaList_MultiInclude();
    void test_api_runtime_AdaList_Length();
    void test_api_runtime_AdaList_Append();
    void test_api_runtime_AdaList_Insert();
    void test_api_runtime_AdaList_Concat();
    void test_api_runtime_AdaList_Contains();
    void test_api_runtime_AdaList_IndexOf();
    void test_api_runtime_AdaList_Flip();
    void test_api_runtime_AdaList_Flipped();
    void test_api_runtime_AdaList_Clear();
    void test_api_runtime_AdaList_Clear_Append();


    // invoke API
    void test_api_runtime_invoke_Fnc1();
    void test_api_runtime_invoke_Fnc2();

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
    void test_error_runtime_symbolLookup();
    void test_error_runtime_divisionByZero();
};

TstParser::TstParser() {}

TstParser::~TstParser() {}

/*-----------------------------------------------------------------------------------------------*\
                                       CORE DATASTRUCTURES
\*-----------------------------------------------------------------------------------------------*/

void TstParser::test_core_SharedString()
{
    NdaState state;
    // Test "Copy on Write"
    NdaVariant s1;

    QCOMPARE(s1.refCount(), 0);

    s1.fromString(state.typeByName("string"),"");
    QCOMPARE(s1.refCount(), 1);
    QCOMPARE(s1.toString(), "");

    s1.fromString(state.typeByName("string"),"1");
    QCOMPARE(s1.refCount(), 1);
    QCOMPARE(s1.toString(), "1");

    NdaVariant s2(s1);
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
        NdaVariant s3;
        QCOMPARE(s1.refCount(), 1);
        QCOMPARE(s3.refCount(), 0);

        s3 = s1;
        QCOMPARE(s1.refCount(), 2);
        QCOMPARE(s3.refCount(), 2);
    }
    QCOMPARE(s1.refCount(), 1);
}
//-------------------------------------------------------------------------------------------------
void TstParser::test_core_NumericLiterals()
{
    QVERIFY(NdaVariant::numericType("0_n") == Nda::Natural);
    QVERIFY(NdaVariant::numericType("0_u") == Nda::Supernatural);
    QVERIFY(NdaVariant::numericType("0_d") == Nda::Number);
    QVERIFY(NdaVariant::numericType("0_b") == Nda::Byte);

    QVERIFY(NdaVariant::numericType("0_z") == Nda::Undefined);


    QVERIFY(NdaVariant::numericType("0")   == Nda::Natural);
    QVERIFY(NdaVariant::numericType("42")  == Nda::Natural);

    QVERIFY(NdaVariant::numericType("0.5")    == Nda::Number);
    QVERIFY(NdaVariant::numericType("1.0E5")    == Nda::Number);
    QVERIFY(NdaVariant::numericType("1.0e5")    == Nda::Number);
    QVERIFY(NdaVariant::numericType("4.2E-5") == Nda::Number);

    QVERIFY(NdaVariant::numericType("2#1000_0100#") == Nda::Natural);
    QVERIFY(NdaVariant::numericType("2#1000#")      == Nda::Natural);
    QVERIFY(NdaVariant::numericType("16#F#")        == Nda::Natural);
    QVERIFY(NdaVariant::numericType("2#1000#_b")    == Nda::Byte);

}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_NumericValues()
{
    NdaState   state;
    NdaVariant v;

    // int64_t
    QCOMPARE(v.fromNaturalLiteral(state.typeByName("natural"),"123_456"),true);
    QCOMPARE(v.type(), Nda::Natural);

    // double
    QCOMPARE(v.fromNumberLiteral(state.typeByName("number"),"1.23E+10"),true);
    QCOMPARE(v.type(), Nda::Number);

    QCOMPARE(v.fromNumberLiteral(state.typeByName("number"),"1.2_34"),true);
    QCOMPARE(v.type(), Nda::Number);

    // uint64_t (Base 16)
    QCOMPARE(v.fromSNaturalLiteral(state.typeByName("supernatural"),"16#1F#"),true);
    QCOMPARE(v.type(), Nda::Supernatural);

    // uint64_t (Base 2)
    QCOMPARE(v.fromSNaturalLiteral(state.typeByName("supernatural"),"2#1011_0001#"),true);
    QCOMPARE(v.type(), Nda::Supernatural);

    // uint64_t (Maximalwert)
    QCOMPARE(v.fromSNaturalLiteral(state.typeByName("supernatural"),"18446744073709551615"),true);
    QCOMPARE(v.type(), Nda::Supernatural);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_NumericValues_Invalid()
{
    // passes NdaVariant::numericType() but are invalid anyway:

    NdaState   state;
    NdaVariant v;

    // uint64_t (too long)
    QCOMPARE(v.fromSNaturalLiteral(state.typeByName("supernatural"),"18446744073709551616"),false);
    QCOMPARE(v.type(), Nda::Undefined);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_VariantToString_Boolean()
{
    NdaState state;

    NdaVariant t;
    t.fromBool(state.booleanType(), true);
    QCOMPARE(t.toString(), std::string("true"));

    NdaVariant f;
    f.fromBool(state.booleanType(), false);
    QCOMPARE(f.toString(), std::string("false"));
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_VariantToString_Byte()
{
    NdaState state;

    NdaVariant v;
    v.fromByte(state.typeByName("byte"), 42);
    QCOMPARE(v.toString(), std::string("42"));
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_Assignment()
{
    NdaState   state;
    NdaVariant v1;
    NdaVariant v2;


    // Number -> Any
    v1.initType(state.typeByName("any"));

    v2.fromNumber(state.typeByName("number"), 42.23);
    QCOMPARE(v1.assign(v2),true);

    // Natural -> Any
    v1.initType(state.typeByName("any"));
    v2.fromNatural(state.typeByName("natural"),42);
    QCOMPARE(v1.assign(v2),true);

    // Supernatural -> Any
    v1.initType(state.typeByName("any"));
    v2.fromSNatural(state.typeByName("supernatural"),(uint64_t)42);
    QCOMPARE(v1.assign(v2),true);

    // String -> Any
    v1.initType(state.typeByName("any"));
    v2.fromString(state.typeByName("string"),"neoAda");
    QCOMPARE(v1.assign(v2),true);
    QCOMPARE(v1.toString(), v2.toString());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_References()
{
    NdaState   state;
    NdaVariant v1;
    NdaVariant r1;
    r1.fromReference(state.typeByName("reference"),&v1);
    v1.fromString(state.typeByName("string"), "hello");

    QCOMPARE(r1.type(), Nda::Type::String);
    QCOMPARE(r1.toString(), v1.toString());

    NdaVariant r2 = r1; // copy reference
    NdaVariant v2;
    v2.fromString(state.typeByName("string"),"world");

    r2.assign(v2);

    QCOMPARE(v1.toString(), v2.toString());
    QCOMPARE(r1.toString(), v1.toString());
    QCOMPARE(r2.toString(), v1.toString());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_List_COW()
{
    NdaState   state;
    NdaVariant vs;
    vs.fromString(state.typeByName("string"),"1");

    { // append, insert remove
        NdaVariant a1;
        a1.initType(state.typeByName("list"));

        QCOMPARE(a1.lengthOperator(), 0);
        a1.appendToList(vs);
        QCOMPARE(a1.lengthOperator(), 1);
        QVERIFY(a1.readAccess(0).toString() == "1");

        vs.fromString(state.typeByName("string"), "0");
        a1.insertIntoList(0,vs);
        QVERIFY(a1.readAccess(0).toString() == "0");
        QVERIFY(a1.readAccess(1).toString() == "1");
        a1.takeFromList(0);
        QVERIFY(a1.readAccess(0).toString() == "1");
        a1.takeFromList(0);
        QCOMPARE(a1.lengthOperator(), 0);
    }

    { // shared list
        NdaVariant a1;
        a1.initType(state.typeByName("list"));

        {
            NdaVariant a2;
            a2.initType(state.typeByName("list"));

            vs.fromString(state.typeByName("string"),"a2");
            a2.appendToList(vs);

            QCOMPARE(a1.listSize(), 0);
            QCOMPARE(a2.listSize(), 1);

            a1 = a2;
        } // destroy a2

        QCOMPARE(a1.listSize(), 1);
        QVERIFY(a1.readAccess(0).toString() == "a2");
    }

    { // copy-on-write: source changes
        NdaVariant a1;
        a1.initType(state.typeByName("list"));

        NdaVariant a2;
        a2.initType(state.typeByName("list"));

        vs.fromString(state.typeByName("string"),"0");
        a1.appendToList(vs);

        a2 = a1;
        QVERIFY(a1.readAccess(0).toString() == "0");
        QVERIFY(a2.readAccess(0).toString() == "0");

        vs.fromString(state.typeByName("string"),"1");
        a1.insertIntoList(0,vs);
        QVERIFY(a1.readAccess(0).toString() == "1");
        QVERIFY(a2.readAccess(0).toString() == "0");
        QCOMPARE(a1.listSize(), 2);
        QCOMPARE(a2.listSize(), 1);
    }

    { // copy-on-write: target changes
        NdaVariant a1;
        a1.initType(state.typeByName("list"));

        NdaVariant a2;
        a2.initType(state.typeByName("list"));

        vs.fromString(state.typeByName("string"),"0");
        a1.appendToList(vs);

        a2 = a1;
        QVERIFY(a1.readAccess(0).toString() == "0");
        QVERIFY(a2.readAccess(0).toString() == "0");

        vs.fromString(state.typeByName("string"),"1");
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
    NdaState   state;
    NdaVariant vs;
    vs.fromString(state.typeByName("string"),"1");

    {
        NdaVariant a1;
        a1.initType(state.typeByName("list"));
        a1.appendToList(vs);

        NdaVariant r1;
        r1.fromReference(state.typeByName("reference"),&a1);
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
    NdaState state;

    NdaVariant vs;
    vs.fromString(state.typeByName("string"),"0");

    NdaVariant a1;
    a1.initType(state.typeByName("list"));
    a1.appendToList(vs);

    QVERIFY(a1.containsInList(vs) == true);

    vs.fromNumber(state.typeByName("number"),0);
    QVERIFY(a1.containsInList(vs) == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_List_Concat()
{
    NdaState state;

    NdaVariant vs;
    vs.fromString(state.typeByName("string"),"0");

    NdaVariant a1;
    a1.initType(state.typeByName("list"));
    a1.appendToList(vs);

    vs.fromString(state.typeByName("string"),"1");
    NdaVariant a2;
    a2.initType(state.typeByName("list"));
    a2.appendToList(vs);

    NdaVariant a3 = a1.concat(a2);

    vs.fromString(state.typeByName("string"),"0");
    QVERIFY(a3.containsInList(vs) == true);
    vs.fromString(state.typeByName("string"),"1");
    QVERIFY(a3.containsInList(vs) == true);

    QVERIFY(a1.listSize() == 1);
    QVERIFY(a2.listSize() == 1);
    QVERIFY(a3.listSize() == 2);
}

void TstParser::test_core_Dict_COW()
{
    NdaState   state;
    NdaVariant vkey;
    vkey.fromString(state.typeByName("string"),"1");
    NdaVariant vvalue;
    vvalue.fromString(state.typeByName("string"),"2");

    { // append, insert remove
        NdaVariant a1;
        a1.initType(state.typeByName("dict"));

        QCOMPARE(a1.dictSize(), 0);

        a1.appendToDict(vkey,vvalue);

        QCOMPARE(a1.dictSize(), 1);
        QCOMPARE(a1.contains(vkey), true);
        QCOMPARE(a1.contains(vvalue), false);

        QCOMPARE(a1.writeDictAccess(vkey).toString(),vvalue.toString());

        a1.takeFromDict(vkey);
    }

    { // shared dict
        NdaVariant a1;
        a1.initType(state.typeByName("dict"));

        {
            NdaVariant a2;
            a2.initType(state.typeByName("dict"));
            a2.appendToDict(vkey, vvalue);

            QCOMPARE(a1.dictSize(), 0);
            QCOMPARE(a2.dictSize(), 1);

            QCOMPARE(a1.refCount(), 1);
            QCOMPARE(a2.refCount(), 1);

            a1 = a2;
            QCOMPARE(a1.dictSize(), 1);
            QCOMPARE(a1.refCount(), 2);
            QCOMPARE(a2.refCount(), 2);
        } // destroy a2

        QCOMPARE(a1.dictSize(), 1);
        QCOMPARE(a1.writeDictAccess(vkey).toString(),vvalue.toString());
    }

    { // copy-on-write: source changes
        NdaVariant a1;
        a1.initType(state.typeByName("dict"));

        NdaVariant a2;
        a2.initType(state.typeByName("dict"));

        vvalue.fromString(state.typeByName("string"),"2");
        a1.appendToDict(vkey, vvalue);

        QCOMPARE(a1.refCount(),1);
        QCOMPARE(a2.refCount(),1);

        a2 = a1;

        QCOMPARE(a1.refCount(),2);
        QCOMPARE(a2.refCount(),2);

        vvalue.fromString(state.typeByName("string"),"42");
        a1.appendToDict(vkey, vvalue);

        QCOMPARE(a1.refCount(),1);
        QCOMPARE(a2.refCount(),1);

        // qDebug() << QString::fromStdString(a1.toString()) << QString::fromStdString(a2.toString());

        QVERIFY(a1.writeDictAccess(vkey).toString() == "42");
        QVERIFY(a2.writeDictAccess(vkey).toString() == "2");
    }

    { // copy-on-write: target changes
        NdaVariant a1;
        a1.initType(state.typeByName("dict"));

        NdaVariant a2;
        a2.initType(state.typeByName("dict"));

        vvalue.fromString(state.typeByName("string"),"2");
        a1.appendToDict(vkey, vvalue);

        a2 = a1;

        vvalue.fromString(state.typeByName("string"),"42");
        a2.appendToDict(vkey, vvalue);

        QVERIFY(a1.writeDictAccess(vkey).toString() == "2");
        QVERIFY(a2.writeDictAccess(vkey).toString() == "42");
    }
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_core_Value_CTor()
{
    QVERIFY(NdaValue(23.42).toDouble()   == 23.42);
    QVERIFY(NdaValue(9000000000).toInt64()   == 9000000000);
    QVERIFY(NdaValue("Hello").toString() == "Hello");
    QVERIFY(NdaValue(true).toBool()   == true);
}

/*-----------------------------------------------------------------------------------------------*\
                                       LEXER LITERALS
\*-----------------------------------------------------------------------------------------------*/


//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_Numbers()
{
    NdaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("123 1_000.0 42E+3 16#FF# 2#1010#E+2 0_n 0_u 0_d");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 8);
    QVERIFY(results[0] == "123");
    QVERIFY(results[1] == "1_000.0");
    QVERIFY(results[2] == "42E+3");
    QVERIFY(results[3] == "16#FF#");
    QVERIFY(results[4] == "2#1010#E+2");
    QVERIFY(results[5] == "0_n"); // Natural
    QVERIFY(results[6] == "0_u"); // Supernatural
    QVERIFY(results[7] == "0_d"); // Supernatural
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_OperatorsAndSymbols()
{
    NdaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript(":= => ** <= >= + - * / > < ( ) ;");
    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 14);
    QVERIFY(results[0] == ":=");
    QVERIFY(results[1] == "=>");
    QVERIFY(results[2] == "**"); // Ada95 Power
    QVERIFY(results[3] == "<=");
    QVERIFY(results[4] == ">=");

    QVERIFY(results[5] == "+");
    QVERIFY(results[6] == "-");
    QVERIFY(results[7] == "*");
    QVERIFY(results[8] == "/");
    QVERIFY(results[9] == ">");
    QVERIFY(results[10] == "<");
    QVERIFY(results[11] == "(");
    QVERIFY(results[12] == ")");
    QVERIFY(results[13] == ";");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_lexer_Identifiers()
{
    NdaLexer lexer;
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
    NdaLexer lexer;
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
    NdaLexer lexer;
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
    NdaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("1..10");

    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 3);
    QVERIFY(results[0] == "1");
    QVERIFY(results[1] == "..");
    QVERIFY(results[2] == "10");
}

void TstParser::test_lexer_ArrayInit()
{
    NdaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("[0,1]");

    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 5);
    QVERIFY(results[0] == "[");
    QVERIFY(results[1] == "0");
    QVERIFY(results[2] == ",");
    QVERIFY(results[3] == "1");
    QVERIFY(results[4] == "]");
}

void TstParser::test_lexer_ArrayAccess()
{
    NdaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("x[7] :=");

    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 5);
    QVERIFY(results[0] == "x");
    QVERIFY(results[1] == "[");
    QVERIFY(results[2] == "7");
    QVERIFY(results[3] == "]");
    QVERIFY(results[4] == ":=");
}

void TstParser::test_lexer_DictInit()
{
    NdaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("{0:1,1:2}");

    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 9);;
    QVERIFY(results[0] == "{");
    QVERIFY(results[1] == "0");
    QVERIFY(results[2] == ":");
    QVERIFY(results[3] == "1");
    QVERIFY(results[4] == ",");
    QVERIFY(results[5] == "1");
    QVERIFY(results[6] == ":");
    QVERIFY(results[7] == "2");
    QVERIFY(results[8] == "}");
}

void TstParser::test_lexer_DictAccess()
{
    NdaLexer lexer;
    std::vector<std::string> results;

    lexer.setScript("x{7} :=");

    while (lexer.nextToken())
        results.push_back(lexer.token());

    QVERIFY(results.size() == 5);
    QVERIFY(results[0] == "x");
    QVERIFY(results[1] == "{");
    QVERIFY(results[2] == "7");
    QVERIFY(results[3] == "}");
    QVERIFY(results[4] == ":=");
}

void TstParser::test_lexer_Comments() {
    NdaLexer lexer;
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
    NdaLexer lexer;
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
    NdaLexer lexer;
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
    NdaLexer lexer;
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
    NdaLexer lexer;
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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
void TstParser::test_parser_Declaration5_List_Init1()
{
    std::string script = R"(
        declare x : List := [1,2,3];
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
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
void TstParser::test_parser_Declaration5_List_Init2()
{
    std::string script = R"(
        declare matrix : List := [[1,2,3],[4,5,6],[7,8,9]];
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Declaration, "matrix")
    Node(Identifier, "List")
    Node(ListLiteral, "")
      Node(ListLiteral, "")
        Node(Number, "1")
        Node(Number, "2")
        Node(Number, "3")
      Node(ListLiteral, "")
        Node(Number, "4")
        Node(Number, "5")
        Node(Number, "6")
      Node(ListLiteral, "")
        Node(Number, "7")
        Node(Number, "8")
        Node(Number, "9")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Declaration6_Expression()
{
    std::string script = R"(
        declare x : Natural := n / 7;
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Declaration, "x")
    Node(Identifier, "Natural")
    Node(BinaryOperator, "/")
      Node(Identifier, "n")
      Node(Number, "7")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Declaration10_Dict()
{
    std::string script = R"(
        declare x : Dict;
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Declaration, "x")
    Node(Identifier, "Dict")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_With()
{
    std::string script = R"(
        with Ada.Lovelace;
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(WithAddon, "Ada.Lovelace")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Type()
{
    std::string script = R"(
        type File is Natural;
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(TypeDefinition, "File")
    Node(Identifier, "Natural")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Exception()
{
    std::string script = R"(
        procedure Main() is
        begin
            raise my_exception;
        exception
            when my_exception => print("error");
            when others => print("other");
        end;
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Procedure, "Main")
    Node(Parameters, "")
    Node(Block, "")
      Node(Raise, "")
        Node(Identifier, "my_exception")
      Node(Exception, "")
        Node(ExceptionHandler, "my_exception")
          Node(Block, "")
            Node(FunctionCall, "print")
              Node(Literal, "error")
        Node(ExceptionHandler, "others")
          Node(Block, "")
            Node(FunctionCall, "print")
              Node(Literal, "other")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_ExceptionReraise()
{
    std::string script = R"(
        procedure Main() is
        begin
            raise ConstraintError;
        exception
            when others => raise;
        end;
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Procedure, "Main")
    Node(Parameters, "")
    Node(Block, "")
      Node(Raise, "")
        Node(Identifier, "ConstraintError")
      Node(Exception, "")
        Node(ExceptionHandler, "others")
          Node(Block, "")
            Node(Raise, "")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
    Node(Expression, "")
      Node(FunctionCall, "a")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Primary3_List_Access()
{
    std::string script = R"(
        x[1] := 7;
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Unknown, "[")
      Node(Identifier, "x")
      Node(Number, "1")
    Node(Number, "7")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Primary4_ListInList()
{
    std::string script = R"(
        x := [[]];
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
    Node(ListLiteral, "")
      Node(ListLiteral, "")
)";
    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_Primary5_DictInDict()
{
    std::string script = R"(
        x := {1:{}};
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
    Node(DictLiteral, "")
      Node(Number, "1")
      Node(DictLiteral, "")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Declaration, "x")
    Node(Identifier, "Natural")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Assignment, ":=")
    Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(InstanceMethodCall, "print")
    Node(Identifier, "x")
)";

    std::string currentAST =  ast->serialize();
    QCOMPARE_TRIM(currentAST, expectedAST);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_parser_MethodCall3_Chained()
{
    std::string script = R"(
        start.addDays(3).addSecs(2 * 60 * 60);
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(InstanceMethodCall, "addSecs")
    Node(InstanceMethodCall, "addDays")
      Node(Identifier, "start")
      Node(Number, "3")
    Node(BinaryOperator, "*")
      Node(BinaryOperator, "*")
        Node(Number, "2")
        Node(Number, "60")
      Node(Number, "60")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(WhileLoop, "")
    Node(BinaryOperator, "<")
      Node(Identifier, "x")
      Node(Number, "10")
    Node(Block, "")
      Node(Assignment, ":=")
        Node(Identifier, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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
void TstParser::test_parser_return1()
{
    std::string script = R"(
        return 42;
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
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
void TstParser::test_parser_return2()
{
    std::string script = R"(
        return a & "x";
    )";

    NdaLexer lexer;
    NdaParser parser(lexer);
    auto ast = parser.parse(script);

    std::string expectedAST = R"(
Node(Program, "")
  Node(Return, "")
    Node(BinaryOperator, "&")
      Node(Identifier, "a")
      Node(Literal, "x")
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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

    NdaLexer lexer;
    NdaParser parser(lexer);
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
    NdaState state;

    // enum Type { Undefined, Any, Number, Natural, Supernatural, Boolean, Byte, Character, String, Struct };
    QCOMPARE(state.define("a", "Any"), true);
    QCOMPARE(state.define("b", "Number"), true);
    QCOMPARE(state.define("c", "Natural"), true);
    QCOMPARE(state.define("d", "Supernatural"), true);
    QCOMPARE(state.define("e", "Boolean"), true);
    QCOMPARE(state.define("f", "Byte"), true);
    QCOMPARE(state.define("h", "String"), true);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_state_GlobalScope()
{
    NdaState state;

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
void TstParser::test_interpreter_Declarations1()
{
    //  enum Type { Undefined, Any, Number, Natural, Supernatural, Boolean, Byte, Character, String, Struct };

    std::string script = R"(
        declare x: Natural;

        declare a: Any;
        declare b: Number;
        declare c: Supernatural;
        declare d: Boolean;
        declare e: Byte;
        declare g: String;

    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QCOMPARE(state.typeOf("x"), Nda::Natural);
    QCOMPARE(state.typeOf("z"), Nda::Undefined);

    QCOMPARE(state.typeOf("a"), Nda::Any);
    QCOMPARE(state.typeOf("b"), Nda::Number);
    QCOMPARE(state.typeOf("c"), Nda::Supernatural);
    QCOMPARE(state.typeOf("d"), Nda::Boolean);
    QCOMPARE(state.typeOf("e"), Nda::Byte);
    QCOMPARE(state.typeOf("g"), Nda::String);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Declarations2()
{
    //  enum Type { Undefined, Any, Number, Natural, Supernatural, Boolean, Byte, Character, String, Struct };

    std::string script = R"(
        declare x: Natural := 3;
        declare y: Natural := x-2;

        return y;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_WithAddon()
{
    std::string script = R"(
        with Ada.Lovelace;
        with Ada.String;
        with My.Addon;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.onWith([&](std::string addonName) {
        results.push_back(addonName);
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 3);
    QVERIFY(results[0] == "ada.lovelace");
    QVERIFY(results[1] == "ada.string");
    QVERIFY(results[2] == "my.addon");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ProcedureCall()
{
    std::string script = R"(
        function square(x : Natural) return Natural is
        begin
            return x * x;
        end;

        print("hello NeoAda");
        print(42);
        print(3 & "^2 = " & square(3));
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        results.push_back(args.at("message").toString());
        return true;
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 3);
    QVERIFY(results[0] == "hello NeoAda");
    QVERIFY(results[1] == "42");
    QVERIFY(results[2] == "3^2 = 9");
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

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        results.push_back(args.at("message").toString());
        return true;
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

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        results.push_back(args.at("message").toString());
        return true;
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 1);
    QVERIFY(results[0] == "x<=5");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ifElseIfStatement()
{
    std::string script = R"(
        declare x : Number := 8.0;
        if x > 10 then
            print("x>5");
        elsif x > 7 then
            print("x>7");
        else
            print("x<=7");
        end if;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        results.push_back(args.at("message").toString());
        return true;
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 1);
    QVERIFY(results[0] == "x>7");
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

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        results.push_back(args.at("message").toString());
        return true;
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

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        results.push_back(args.at("message").toString());
        return true;
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

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        results.push_back(args.at("message").toString());
        return true;
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

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        results.push_back(args.at("message").toString());
        return true;
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

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

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

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

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

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toBool(nullptr) == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Return4()
{
    std::string script = R"(
        return "NeoAda";
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "NeoAda");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Return_ForLoop()
{
    std::string script = R"(
    for i in 1..10 loop
        return i;
    end loop;
    return 99;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Return5_Natural()
{
    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse("return 0_n;");
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.type() == Nda::Natural);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Return6_Supernatural()
{
    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse("return 0_u;");
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.type() == Nda::Supernatural);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Return7_Number()
{
    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse("return 0_d;");
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.type() == Nda::Number);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Return8_Byte()
{
    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse("return 0_b;");
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.type() == Nda::Byte);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ExceptionHandled()
{
    std::string script = R"(
        function Test() return Natural is
        begin
            raise my_exception;
            return 0;
        exception
            when my_exception => return 42;
        end;

        return Test();
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "42");
    QVERIFY(state.unhandledException().empty());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ExceptionUnhandled()
{
    std::string script = R"(
        function Test() return Natural is
        begin
            raise my_exception;
            return 0;
        end;

        return Test();
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    interpreter.execute(ast);

    QVERIFY(state.unhandledException() == "my_exception");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ExceptionCatchAll()
{
    std::string script = R"(
        function Test() return Natural is
        begin
            raise my_exception;
            return 0;
        exception
            when others => return 42;
        end;

        return Test();
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "42");
    QVERIFY(state.unhandledException().empty());
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ExceptionConstraintError()
{
    std::string script = R"(
        function Test() return Natural is
        begin
            return 42 / 0;
        exception
            when ConstraintError => return 23;
        end;

        return Test();
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "23");
    QVERIFY(state.unhandledException().empty());

    std::string boundsScript = R"(
        function Test() return Natural is
        begin
            declare xs : List := [1, 2, 3];
            return xs[3];
        exception
            when ConstraintError => return 42;
        end;

        return Test();
    )";

    NdaLexer       boundsLexer;
    NdaParser      boundsParser(boundsLexer);
    NdaState       boundsState;
    NdaInterpreter boundsInterpreter(&boundsState);

    auto boundsAst = boundsParser.parse(boundsScript);
    auto boundsRet = boundsInterpreter.execute(boundsAst);

    QVERIFY(boundsRet.toString() == "42");
    QVERIFY(boundsState.unhandledException().empty());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ExceptionProgramError()
{
    std::string script = R"(
        function Test() return Natural is
        begin
            declare x : Natural := "NeoAda";
            return 0;
        exception
            when ProgramError => return 17;
        end;

        return Test();
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "17");
    QVERIFY(state.unhandledException().empty());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ExceptionReraiseSpecific()
{
    std::string script = R"(
        function Inner() return Natural is
        begin
            return 42 / 0;
        exception
            when ConstraintError => raise ConstraintError;
        end;

        function Outer() return Natural is
        begin
            return Inner();
        exception
            when ConstraintError => return 31;
        end;

        return Outer();
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "31");
    QVERIFY(state.unhandledException().empty());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_ExceptionReraiseAnonymous()
{
    std::string script = R"(
        function Inner() return Natural is
        begin
            return 42 / 0;
        exception
            when others => raise;
        end;

        function Outer() return Natural is
        begin
            return Inner();
        exception
            when ConstraintError => return 37;
        end;

        return Outer();
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);
    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "37");
    QVERIFY(state.unhandledException().empty());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_CustomType_TypeIs()
{
    std::string script = R"(
        type File is Natural;
        declare x : File;

        x := 42;
        return x;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);


    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_CustomType_Procedure()
{
    std::string script = R"(
        type File is Natural;

        function File:myVersion return Natural is
        begin
            return 42;
        end;

        return File:myVersion();
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);


    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Volatile_CTor()
{
    std::string script = R"(
        volatile x : Natural;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.onVolatileCtor([&](const std::string &name,  NdaVariant& val) -> void {
        results.push_back(name);
        results.push_back(val.type() == Nda::Natural ? "Natural" : "?");
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 2);
    QVERIFY(results[0] == "x");
    QVERIFY(results[1] == "Natural");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Volatile_Read()
{
    std::string script = R"(
        volatile x : Natural;
        return x;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.onVolatileRead("x", [&](NdaVariant& val) -> bool {
        val.setNatural(42);
        return true;
    });

    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "42");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Volatile_ReadFailed()
{
    std::string script = R"(
        volatile x : Natural;
        x := 42;
        return x;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.onVolatileRead("x", [&](NdaVariant&) -> bool {
        return false;
    });

    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "42");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Volatile_ReadDict()
{
    std::string script = R"(
        volatile model : Dict;

        model{"sensor1"} := 0;

        return model{"sensor1"};
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.onVolatileRead("model", [&](const NdaVariant &index, NdaVariant& val) -> bool {
        if (index.toString() == "sensor1") {
            val.setNatural(42);
            return true;
        }
        return false;
    });

    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "42");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Volatile_Write()
{
    std::string script = R"(
        volatile x : Natural;
        x := 42;
        return x;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    int64_t modelValue = 0;
    int writeCount = 0;
    state.onVolatileWrite("x", [&](const NdaVariant &value) -> bool {
        bool ok = false;
        modelValue = value.toInt64(&ok);
        if (!ok)
            return false;
        ++writeCount;
        return true;
    });

    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toInt64() == 42);
    QVERIFY(modelValue == 42);
    QVERIFY(writeCount == 1);
    QVERIFY(state.unhandledException().empty());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Volatile_WriteRejected()
{
    std::string script = R"(
        function Test() return Natural is
        begin
            volatile x : Natural;
            x := 42;
            return x;
        exception
            when ProgramError => return 17;
        end;

        return Test();
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    int64_t attemptedValue = 0;
    int writeCount = 0;
    state.onVolatileWrite("x", [&](const NdaVariant &value) -> bool {
        bool ok = false;
        attemptedValue = value.toInt64(&ok);
        if (!ok)
            return false;
        ++writeCount;
        return false;
    });

    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toInt64() == 17);
    QVERIFY(attemptedValue == 42);
    QVERIFY(writeCount == 1);
    QVERIFY(state.unhandledException().empty());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Volatile_WriteDict()
{
    std::string script = R"(
        volatile model : Dict;
        model{"sensor1"} := 42;
        return model{"sensor1"};
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::string modelKey;
    int64_t modelValue = 0;
    int writeCount = 0;
    state.onVolatileWrite("model", [&](const NdaVariant &index, const NdaVariant &value) -> bool {
        bool ok = false;
        modelKey = index.toString();
        modelValue = value.toInt64(&ok);
        if (!ok)
            return false;
        ++writeCount;
        return true;
    });

    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toInt64() == 42);
    QVERIFY(modelKey == "sensor1");
    QVERIFY(modelValue == 42);
    QVERIFY(writeCount == 1);
    QVERIFY(state.unhandledException().empty());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_Volatile_WriteDictRejected()
{
    std::string script = R"(
        function Test() return Natural is
        begin
            volatile model : Dict;
            model{"sensor1"} := 42;
            return model{"sensor1"};
        exception
            when ProgramError => return 17;
        end;

        return Test();
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::string attemptedKey;
    int64_t attemptedValue = 0;
    int writeCount = 0;
    state.onVolatileWrite("model", [&](const NdaVariant &index, const NdaVariant &value) -> bool {
        bool ok = false;
        attemptedKey = index.toString();
        attemptedValue = value.toInt64(&ok);
        if (!ok)
            return false;
        ++writeCount;
        return false;
    });

    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toInt64() == 17);
    QVERIFY(attemptedKey == "sensor1");
    QVERIFY(attemptedValue == 42);
    QVERIFY(writeCount == 1);
    QVERIFY(state.unhandledException().empty());
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_interpreter_static_method()
{
    std::string script = R"(
        return string:number(10);
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;

    state.bindFnc("string","number",{{"n", "natural", Nda::InMode}}, [&](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        std::ostringstream os;
        os << args.at("n").toInt64();

        ret.fromString(state.typeByName("string"),os.str());
        return true;
    });


    auto ret = interpreter.execute(ast);

    QVERIFY(ret.toString() == "10");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Literals()
{
    NdaState state;
    QVERIFY(NeoAda::evaluate("return true;",state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return false;",state).toBool() == false);
    QVERIFY(NeoAda::evaluate("return 42;",state).toInt64() == 42);
    QVERIFY(NeoAda::evaluate("return -1;",state).toInt64() == -1);

    QVERIFY(NeoAda::evaluate("return  10_000;",state).toInt64() == +10000);
    QVERIFY(NeoAda::evaluate("return -10_000;",state).toInt64() == -10000);
    QVERIFY(NeoAda::evaluate("return +10_000;",state).toInt64() == +10000);

    QVERIFY(NeoAda::evaluate("return  2#1001_1000#;",state).toInt64() ==  152);

    QVERIFY(NeoAda::evaluate("return \"NeoAda\";",state).toString() == "NeoAda");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Length()
{
    NdaState state;
    QVERIFY(NeoAda::evaluate("return  #1;", state).toInt64()   == 1);
    QVERIFY(NeoAda::evaluate("return  #1.1;", state).toInt64() == 1);
    QVERIFY(NeoAda::evaluate("return #\"\";", state).toInt64() == 0);
    QVERIFY(NeoAda::evaluate("return #\"NeoAda\";", state).toInt64() == 6);

    QVERIFY(NeoAda::evaluate("return #[];", state).toInt64()      == 0);
    QVERIFY(NeoAda::evaluate("return #[1];", state).toInt64()     == 1);
    QVERIFY(NeoAda::evaluate("return #[1,2,3];", state).toInt64() == 3);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Equal()
{
    NdaState state;
    QVERIFY(NeoAda::evaluate("return true = true;", state).toBool()  == true);
    QVERIFY(NeoAda::evaluate("return true = false;", state).toBool() == false);
    QVERIFY(NeoAda::evaluate("return 42 = 42;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42.5 = 42.5;", state).toBool()  == true);
    QVERIFY(NeoAda::evaluate("return 42 = 23;", state).toBool()      == false);

    QVERIFY(NeoAda::evaluate("return \"NeoAda\" = \"NeoAda\";", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return \"NeoAda\" = \"neoada\";", state).toBool() == false); // case insensitive as in Ada95


    QVERIFY(NeoAda::evaluate("return (true = true);", state ).toBool() == true);
    QVERIFY(NeoAda::evaluate("return (true = false);", state).toBool() == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_NotEqual()
{
    NdaState state;

    // Natural
    QVERIFY(NeoAda::evaluate("return 42   <> 42;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 42   <> 23;", state).toBool()      == true);

    // Supernatural
    QVERIFY(NeoAda::evaluate("return 42_u <> 42_u;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 42_u <> 23_u;", state).toBool()      == true);

    // Byte
    QVERIFY(NeoAda::evaluate("return 42_b <> 42_b;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 42_b <> 23_b;", state).toBool()      == true);

    // Number
    QVERIFY(NeoAda::evaluate("return 42.5 <> 42.5;", state).toBool()  == false);
    QVERIFY(NeoAda::evaluate("return 42.4 <> 42.5;", state).toBool()  == true);

    // Boolean
    QVERIFY(NeoAda::evaluate("return true <> true;", state).toBool()  == false);
    QVERIFY(NeoAda::evaluate("return true <> false;", state).toBool() == true);

    // String
    QVERIFY(NeoAda::evaluate("return \"NeoAda\" <> \"NeoAda\";", state).toBool() == false);
    QVERIFY(NeoAda::evaluate("return \"NeoAda\" <> \"neoada\";", state).toBool() == true); // case insensitive as in Ada95
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_LessThan()
{
    NdaState state;

    // Natural
    QVERIFY(NeoAda::evaluate("return 42 < 42;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23 < 42;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42 < 23;", state).toBool()      == false);

    // Natural/Supernatural
    QVERIFY(NeoAda::evaluate("return 42 < 42_u;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23 < 42_u;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42 < 23_u;", state).toBool()      == false);

    // Natural/Byte
    QVERIFY(NeoAda::evaluate("return 42 < 42_b;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23 < 42_b;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42 < 23_b;", state).toBool()      == false);

    // Natural/Number
    QVERIFY(NeoAda::evaluate("return 42 < 42_d;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23 < 42_d;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42 < 23_d;", state).toBool()      == false);


    // Supernatural
    QVERIFY(NeoAda::evaluate("return 42_u < 42_u;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23_u < 42_u;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42_u < 23_u;", state).toBool()      == false);

    // Supernatural/Natural
    QVERIFY(NeoAda::evaluate("return 42_u < 42_n;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23_u < 42_n;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42_u < 23_n;", state).toBool()      == false);

    // Supernatural/Byte
    QVERIFY(NeoAda::evaluate("return 42_u < 42_b;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23_u < 42_b;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42_u < 23_b;", state).toBool()      == false);

    // Byte
    QVERIFY(NeoAda::evaluate("return 42_b < 42_b;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23_b < 42_b;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42_b < 23_b;", state).toBool()      == false);

    // Byte/Natural
    QVERIFY(NeoAda::evaluate("return 42_b < 42_n;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23_b < 42_n;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42_b < 23_n;", state).toBool()      == false);

    // Byte/Supernatural
    QVERIFY(NeoAda::evaluate("return 42_b < 42_u;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23_b < 42_u;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42_b < 23_u;", state).toBool()      == false);

    // Byte/Number
    QVERIFY(NeoAda::evaluate("return 42_b < 42_d;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23_b < 42_d;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42_b < 23_d;", state).toBool()      == false);


    // Number
    QVERIFY(NeoAda::evaluate("return 42.0 < 42.0;", state).toBool()      == false);
    QVERIFY(NeoAda::evaluate("return 23.0 < 42.0;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42.0 < 23.0;", state).toBool()      == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_SignedUnsignedComparisonEdges()
{
    NdaState state;

    // Negative Natural vs unsigned Supernatural
    QVERIFY(NeoAda::evaluate("return -5_n < 10_u;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return -5_n <= 10_u;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return -5_n > 10_u;", state).toBool() == false);
    QVERIFY(NeoAda::evaluate("return -5_n >= 10_u;", state).toBool() == false);
    QVERIFY(NeoAda::evaluate("return -5_n = 10_u;", state).toBool() == false);
    QVERIFY(NeoAda::evaluate("return -5_n <> 10_u;", state).toBool() == true);

    QVERIFY(NeoAda::evaluate("return 10_u > -5_n;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 10_u >= -5_n;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 10_u < -5_n;", state).toBool() == false);
    QVERIFY(NeoAda::evaluate("return 10_u <= -5_n;", state).toBool() == false);

    // Equal values across signed, unsigned and byte types
    QVERIFY(NeoAda::evaluate("return 0_n = 0_u;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 0_u = 0_n;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 255_b = 255_u;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 255_u = 255_b;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 255_b <> 255_u;", state).toBool() == false);

    // Boundary around byte range
    QVERIFY(NeoAda::evaluate("return 255_b < 256_u;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 256_u > 255_b;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return -1_n < 0_b;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 0_b > -1_n;", state).toBool() == true);

    // Number mixed with integer-like types
    QVERIFY(NeoAda::evaluate("return -5.0 = -5_n;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return -5_n = -5.0;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return -5.5 < -5_n;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 10_u > 9.5;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 9.5 < 10_u;", state).toBool() == true);

    // Larger unsigned values should compare numerically, not via signed wraparound.
    QVERIFY(NeoAda::evaluate("return 4294967296_u > -1_n;", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return -1_n < 4294967296_u;", state).toBool() == true);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_LessEqualThan()
{
    NdaState state;

    // Natural
    QVERIFY(NeoAda::evaluate("return 42 <= 42;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 23 <= 42;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42 <= 23;", state).toBool()      == false);

    // Supernatural
    QVERIFY(NeoAda::evaluate("return 42_u <= 42_u;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 23_u <= 42_u;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42_u <= 23_u;", state).toBool()      == false);

    // Byte
    QVERIFY(NeoAda::evaluate("return 42_b <= 42_b;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 23_b <= 42_b;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42_b <= 23_b;", state).toBool()      == false);

    // Number
    QVERIFY(NeoAda::evaluate("return 42.0 <= 42.0;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 23.0 <= 42.0;", state).toBool()      == true);
    QVERIFY(NeoAda::evaluate("return 42.0 <= 23.0;", state).toBool()      == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_ConcatString()
{
    NdaState state;
    QVERIFY(NeoAda::evaluate("return \"Neo\" & \"Ada\";", state).toString()            == "NeoAda");
    QVERIFY(NeoAda::evaluate("return \"Neo\" & \" \" & \"Ada\";", state).toString()    == "Neo Ada");
    QVERIFY(NeoAda::evaluate("return \"Neo\" & \"Ada\" = \"NeoAda\";", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("declare neo: String := \"Neo\"; return (neo & \"Ada\") = \"NeoAda\";", state).toBool() == true);
    QVERIFY(NeoAda::evaluate("return 3 & \"x\";", state).toString() == "3x");
    QVERIFY(NeoAda::evaluate("return \"x\" & 3;", state).toString() == "x3");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Modulo()
{
    NdaState state;
    QVERIFY(NeoAda::evaluate("return 42 mod 21;", state).toInt64() == 0);
    QVERIFY(NeoAda::evaluate("return 42 mod 20;", state).toInt64() == 2);
    QVERIFY(NeoAda::evaluate("return  5 mod 6;", state).toInt64()  == 5);
    QVERIFY(NeoAda::evaluate("return  4 mod 4;", state).toInt64()  == 0);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Multiply()
{
    NdaState state;
    QVERIFY(NeoAda::evaluate("return 2   * 21;", state).toInt64()   == 42);
    QVERIFY(NeoAda::evaluate("return 2.0 * 21.0;", state).toInt64() == 42);
    QVERIFY(NeoAda::evaluate("return -42 * -1;", state).toInt64()   == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Relations()
{
    NdaState state;

    // AND
    QVERIFY(NeoAda::evaluate("return true  and true;", state).toBool()   == true);
    QVERIFY(NeoAda::evaluate("return false and true;", state).toBool()   == false);
    QVERIFY(NeoAda::evaluate("return true  and false;", state).toBool()  == false);
    QVERIFY(NeoAda::evaluate("return false and false;", state).toBool()  == false);

    // OR
    QVERIFY(NeoAda::evaluate("return true  or true;", state).toBool()   == true);
    QVERIFY(NeoAda::evaluate("return false or true;", state).toBool()   == true);
    QVERIFY(NeoAda::evaluate("return true  or false;", state).toBool()  == true);
    QVERIFY(NeoAda::evaluate("return false or false;", state).toBool()  == false);

    // XOR
    QVERIFY(NeoAda::evaluate("return true  xor true;", state).toBool()   == false);
    QVERIFY(NeoAda::evaluate("return false xor true;", state).toBool()   == true);
    QVERIFY(NeoAda::evaluate("return true  xor false;", state).toBool()  == true);
    QVERIFY(NeoAda::evaluate("return false xor false;", state).toBool()  == false);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_List_Init()
{
    std::string script = R"(
        declare x : List := [1,2,3];
        print(x);
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        results.push_back(args.at("message").toString());
        return true;
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 1);
    QVERIFY(results[0] == "[1,2,3]");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_List_Read1()
{
    std::string script = R"(
        declare x : List := [1,42,3];
        print(x[1]);
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    std::vector<std::string> results;
    state.bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        results.push_back(args.at("message").toString());
        return true;
    });

    interpreter.execute(ast);

    QVERIFY(results.size() == 1);
    QVERIFY(results[0] == "42");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_List_Read2()
{
    std::string script = R"(
        declare x : List := [1,42,3];
        declare y : any  := x[1];

        return y;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    auto y = interpreter.execute(ast);

    QVERIFY(y.toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_List_Write()
{
    std::string script = R"(
        declare x : List := [1,2,3];

        x[0] := 42;
        x[1] := 43;
        x[2] := 44;

        return x;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    auto ret = interpreter.execute(ast);

    QVERIFY(ret.type()     == Nda::List);
    QVERIFY(ret.listSize() == 3);
    QVERIFY(ret.readAccess(0).toString() == "42");
    QVERIFY(ret.readAccess(1).toString() == "43");
    QVERIFY(ret.readAccess(2).toString() == "44");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_List_Swap()
{
    std::string script = R"(
        declare x      : List := [1,2,3];
        declare current: Any  := x[0];
        declare next   : Any  := x[2];

        x[0] := next;
        x[2] := current;

        return x;
    )";

    NdaLexer       lexer;
    NdaParser      parser(lexer);
    NdaState       state;
    NdaInterpreter interpreter(&state);

    auto ast = parser.parse(script);

    auto ret = interpreter.execute(ast);

    QVERIFY(ret.type()     == Nda::List);
    QVERIFY(ret.listSize() == 3);
    QVERIFY(ret.readAccess(0).toString() == "3");
    QVERIFY(ret.readAccess(1).toString() == "2");
    QVERIFY(ret.readAccess(2).toString() == "1");
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_Init()
{
    std::string script = R"(
        with Ada.Bytes;

        declare data : Bytes;
        return data.length();
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.type() == Nda::Natural);
    QVERIFY(ret.toInt64() == 0);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_RejectMethodWithoutAddon()
{
    std::string script = R"(
        declare data : Bytes;
        return data.length();
    )";

    NdaRuntime r;
    NdaException ex;
    r.runScript(script, &ex);

    QVERIFY(ex.code() == Nada::Error::UnknownSymbol);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_AppendReadWrite()
{
    std::string script = R"(
        with Ada.Bytes;

        declare data : Bytes;
        data.append(65_b);
        data[0] := 66_b;
        return data[0];
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.type() == Nda::Byte);
    QVERIFY(ret.toInt64() == 66);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_Clear()
{
    std::string script = R"(
        with Ada.Bytes;

        declare data : Bytes;
        data.append(65_b);
        data.append(66_b);
        declare before : Natural := data.length();
        data.clear();
        return before * 10 + data.length();
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 20);
}



//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_AppendBytesContainsIndexOf()
{
    std::string script = R"(
        with Ada.Bytes;

        declare data : Bytes;
        declare other : Bytes;
        data.append(1_b);
        data.append(2_b);
        other.append(3_b);
        other.append(4_b);
        data.append(other);
        if data.contains(3_b) then
            return data.indexOf(4_b) * 10 + data.length();
        end if;
        return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 34);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_InsertRemove()
{
    std::string script = R"(
        with Ada.Bytes;

        declare data : Bytes;
        data.append(1_b);
        data.append(3_b);
        data.insert(1, 2_b);
        data.insert(99, 4_b);
        data.remove(1, 2);
        return data[0] * 10 + data[1];
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 14);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_MidSlicedSlice()
{
    std::string script = R"(
        with Ada.Bytes;

        declare data : Bytes;
        data.append(10_b);
        data.append(20_b);
        data.append(30_b);
        data.append(40_b);
        data.append(50_b);
        declare m : Bytes := data.mid(1, 3);
        declare s : Bytes := data.sliced(2, 2);
        data.slice(1, 2);
        return m.length() * 1000 + m[0] * 10 + s.length() + data.length();
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 3204);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_ChoppedChop()
{
    std::string script = R"(
        with Ada.Bytes;

        declare data : Bytes;
        data.append(1_b);
        data.append(2_b);
        data.append(3_b);
        data.append(4_b);
        declare c : Bytes := data.chopped(2);
        data.chop(1);
        return c.length() * 10 + data.length();
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 23);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_RejectNonByteAppend()
{
    std::string script = R"(
        with Ada.Bytes;

        declare data : Bytes;
        data.append(65_n);
    )";

    NdaRuntime r;
    NdaException ex;
    r.runScript(script, &ex);

    QVERIFY(ex.code() == Nada::Error::UnknownSymbol);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_RejectNonByteWrite()
{
    std::string script = R"(
        with Ada.Bytes;

        declare data : Bytes;
        data.append(65_b);
        data[0] := 66_n;
    )";

    NdaRuntime r;
    NdaException ex;
    r.runScript(script, &ex);

    QVERIFY(ex.code() == Nada::Error::NoError);
    QVERIFY(r.state()->unhandledException() == "programerror");
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Bytes_RejectDictAccessSyntax()
{
    std::string script = R"(
        with Ada.Bytes;

        declare data : Bytes;
        data.append(65_b);
        return data{0};
    )";

    NdaRuntime r;
    NdaException ex;
    r.runScript(script, &ex);

    QVERIFY(ex.code() == Nada::Error::InvalidContainerType);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Dict_Init()
{
    std::string script = R"(
        declare x : Dict := {1:42};
        return x{1};
    )";

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Dict_Append()
{
    std::string script = R"(
        declare x : Dict;
        x{"42"} := 23;
        return x{"42"};
    )";

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toInt64() == 23);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_Dict_Embedded()
{
    std::string script = R"(
        declare x : Dict := {1:{2:42}};
        return x{1}{2};
    )";

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toInt64() == 42);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toBool() == true);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toBool() == false);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toBool() == true);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toBool() == true);

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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toInt64() == 10);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toInt64() == 6);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toInt64() == 5);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toInt64() == 2*42);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toInt64() == 42);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toInt64() == 42);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toString() == "Hello, World");
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toInt64() == 42);
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toString() == "Hello, World");
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toString() == "Hello, World");
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toString() == "Hello, World");
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toString() == "Hello, World");
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

    NdaState state;
    QVERIFY(NeoAda::evaluate(script, state).toString() == "Hello, World");
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_add_Number()
{
    NdaState state;

    // Number + Number
    QVERIFY(NeoAda::evaluate("return 22_d + 20_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_d + 20_d;", state).toInt64() == 42);

    // Number + Natural
    QVERIFY(NeoAda::evaluate("return 22_d + 20_n;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_d + 20_n;", state).toInt64() == 42);

    // Number + Supernatural
    QVERIFY(NeoAda::evaluate("return 22_d + 20_u;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_d + 20_u;", state).toInt64() == 42);

    // Number + Byte
    QVERIFY(NeoAda::evaluate("return 22_d + 20_b;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_d + 20_b;", state).toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_add_Natural()
{
    NdaState state;

    // Natural + Natural
    QVERIFY(NeoAda::evaluate("return 22_n + 20_n;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 22_n + 20_n;", state).toInt64() == 42);

    // Natural + Number
    QVERIFY(NeoAda::evaluate("return 22_n + 20_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_n + 20_d;", state).toInt64() == 42);

    // Natural + Supernatural
    QVERIFY(NeoAda::evaluate("return 22_n + 20_u;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 22_n + 20_u;", state).toInt64() == 42);

    // Natural + Byte
    QVERIFY(NeoAda::evaluate("return 22_n + 20_b;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 22_n + 20_b;", state).toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_add_Supernatural()
{
    NdaState state;

    // Supernatural + Natural
    QVERIFY(NeoAda::evaluate("return 22_u + 20_n;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 22_u + 20_n;", state).toInt64() == 42);

    // Supernatural + Number
    QVERIFY(NeoAda::evaluate("return 22_u + 20_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_u + 20_d;", state).toInt64() == 42);

    // Supernatural + Supernatural
    QVERIFY(NeoAda::evaluate("return 22_u + 20_u;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 22_u + 20_u;", state).toInt64() == 42);

    // Supernatural + Byte
    QVERIFY(NeoAda::evaluate("return 22_u + 20_b;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 22_u + 20_b;", state).toInt64() == 42);

    // unsigned + negative signed
    QVERIFY(NeoAda::evaluate("return 22_u + -2_n;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 22_u + -2_b;", state).type() == Nda::Undefined);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_add_Byte()
{
    NdaState state;

    // Byte*Natural
    QVERIFY(NeoAda::evaluate("return 21_b*2_n;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 21_b*2_n;", state).toInt64() == 42);

    // Byte*Number
    QVERIFY(NeoAda::evaluate("return 21_b*2_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 21_b*2_d;", state).toInt64() == 42);

    // Byte*Supernatural
    QVERIFY(NeoAda::evaluate("return 21_b*2_u;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 21_b*2_u;", state).toInt64() == 42);

    // Byte*Byte
    QVERIFY(NeoAda::evaluate("return 21_b*2_b;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 21_b*2_b;", state).toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_sub_Number()
{
    NdaState state;

    // Number - Number
    QVERIFY(NeoAda::evaluate("return 22_d - 20_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_d - 20_d;", state).toInt64() == 2);

    // Number - Natural
    QVERIFY(NeoAda::evaluate("return 22_d - 20_n;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_d - 20_n;", state).toInt64() == 2);

    // Number - Supernatural
    QVERIFY(NeoAda::evaluate("return 22_d - 20_u;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_d - 20_u;", state).toInt64() == 2);

    // Number - Byte
    QVERIFY(NeoAda::evaluate("return 22_d - 20_b;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_d - 20_b;", state).toInt64() == 2);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_sub_Natural()
{
    NdaState state;

    // Natural - Natural
    QVERIFY(NeoAda::evaluate("return 22_n - 20_n;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 22_n - 20_n;", state).toInt64() == 2);

    // Natural - Number
    QVERIFY(NeoAda::evaluate("return 22_n - 20_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_n - 20_d;", state).toInt64() == 2);

    // Natural - Supernatural
    QVERIFY(NeoAda::evaluate("return 22_n - 20_u;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 22_n - 20_u;", state).toInt64() == 2);

    // Natural - Byte
    QVERIFY(NeoAda::evaluate("return 22_n - 20_b;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 22_n - 20_b;", state).toInt64() == 2);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_sub_Supernatural()
{
    NdaState state;

    // Supernatural - Natural
    QVERIFY(NeoAda::evaluate("return 22_u - 20_n;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 22_u - 20_n;", state).toInt64() == 2);

    // Supernatural - Number
    QVERIFY(NeoAda::evaluate("return 22_u - 20_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 22_u - 20_d;", state).toInt64() == 2);

    // Supernatural - Supernatural
    QVERIFY(NeoAda::evaluate("return 22_u - 20_u;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 22_u - 20_u;", state).toInt64() == 2);

    // Supernatural - Byte
    QVERIFY(NeoAda::evaluate("return 22_u - 20_b;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 22_u - 20_b;", state).toInt64() == 2);

    // unsigned - negative signed / underflow
    QVERIFY(NeoAda::evaluate("return 22_u - -2_n;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 22_u - -2_b;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 2_u - 5_n;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 2_u - 5_u;", state).type() == Nda::Undefined);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_sub_Byte()
{
    NdaState state;

    QVERIFY(NeoAda::evaluate("return 22_b - 20_n;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 22_b - 20_n;", state).toInt64() == 2);
    QVERIFY(NeoAda::evaluate("return 22_b - 20_b;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 22_b - 20_b;", state).toInt64() == 2);
    QVERIFY(NeoAda::evaluate("return 2_b - 5_n;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 2_b - 5_b;", state).type() == Nda::Undefined);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_div_Number()
{
    NdaState state;

    // Number/Number
    QVERIFY(NeoAda::evaluate("return 42_d/2_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 42_d/2_d;", state).toInt64() == 21);

    // Number/Natural
    QVERIFY(NeoAda::evaluate("return 42_d/2_n;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 42_d/2_n;", state).toInt64() == 21);

    // Number/Supernatural
    QVERIFY(NeoAda::evaluate("return 42_d/2_u;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 42_d/2_u;", state).toInt64() == 21);

    // Number/Byte
    QVERIFY(NeoAda::evaluate("return 42_d/2_b;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 42_d/2_b;", state).toInt64() == 21);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_div_Natural()
{
    NdaState state;

    // Natural/Natural
    QVERIFY(NeoAda::evaluate("return 42_n/2_n;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 42_n/2_n;", state).toInt64() == 21);

    // Natural/Number
    QVERIFY(NeoAda::evaluate("return 42_n/2_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 42_n/2_d;", state).toInt64() == 21);

    // Natural/Supernatural
    QVERIFY(NeoAda::evaluate("return 42_n/2_u;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 42_n/2_u;", state).toInt64() == 21);

    // Natural/Byte
    QVERIFY(NeoAda::evaluate("return 42_n/2_b;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 42_n/2_b;", state).toInt64() == 21);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_div_Supernatural()
{
    NdaState state;

    // Supernatural/Natural
    QVERIFY(NeoAda::evaluate("return 42_u/2_n;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 42_u/2_n;", state).toInt64() == 21);

    // Supernatural/Number
    QVERIFY(NeoAda::evaluate("return 42_u/2_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 42_u/2_d;", state).toInt64() == 21);

    // Supernatural/Supernatural
    QVERIFY(NeoAda::evaluate("return 42_u/2_u;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 42_u/2_u;", state).toInt64() == 21);

    // Supernatural/Byte
    QVERIFY(NeoAda::evaluate("return 42_u/2_b;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 42_u/2_b;", state).toInt64() == 21);

    // unsigned / negative signed
    QVERIFY(NeoAda::evaluate("return 42_u/-2_n;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 42_u/-2_b;", state).type() == Nda::Undefined);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_div_Byte()
{
    NdaState state;

    // Byte/Number
    QVERIFY(NeoAda::evaluate("return 42_b/2_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 42_b/2_d;", state).toInt64() == 21);

    // Byte/Natural
    QVERIFY(NeoAda::evaluate("return 42_b/2_n;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 42_b/2_n;", state).toInt64() == 21);

    // Byte/Supernatural
    QVERIFY(NeoAda::evaluate("return 42_b/2_u;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 42_b/2_u;", state).toInt64() == 21);

    // Byte/Byte
    QVERIFY(NeoAda::evaluate("return 42_b/2_b;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 42_b/2_b;", state).toInt64() == 21);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_mul_Number()
{
    NdaState state;

    // Number*Number
    QVERIFY(NeoAda::evaluate("return 21_d*2_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 21_d*2_d;", state).toInt64() == 42);

    // Number*Natural
    QVERIFY(NeoAda::evaluate("return 21_d*2_n;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 21_d*2_n;", state).toInt64() == 42);

    // Number*Supernatural
    QVERIFY(NeoAda::evaluate("return 21_d*2_u;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 21_d*2_u;", state).toInt64() == 42);

    // Number*Byte
    QVERIFY(NeoAda::evaluate("return 21_d*2_b;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 21_d*2_b;", state).toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_mul_Natural()
{
    NdaState state;

    // Natural*Natural
    QVERIFY(NeoAda::evaluate("return 21_n*2_n;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 21_n*2_n;", state).toInt64() == 42);

    // Natural*Number
    QVERIFY(NeoAda::evaluate("return 21_n*2_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 21_n*2_d;", state).toInt64() == 42);

    // Natural*Supernatural
    QVERIFY(NeoAda::evaluate("return 21_n*2_u;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 21_n*2_u;", state).toInt64() == 42);

    // Natural*Byte
    QVERIFY(NeoAda::evaluate("return 21_n*2_b;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 21_n*2_b;", state).toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_mul_Supernatural()
{
    NdaState state;

    // Supernatural*Natural
    QVERIFY(NeoAda::evaluate("return 21_u*2_n;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 21_u*2_n;", state).toInt64() == 42);

    // Supernatural*Number
    QVERIFY(NeoAda::evaluate("return 21_u*2_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 21_u*2_d;", state).toInt64() == 42);

    // Supernatural*Supernatural
    QVERIFY(NeoAda::evaluate("return 21_u*2_u;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 21_u*2_u;", state).toInt64() == 42);

    // Supernatural*Byte
    QVERIFY(NeoAda::evaluate("return 21_u*2_b;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 21_u*2_b;", state).toInt64() == 42);

    // unsigned * negative signed
    QVERIFY(NeoAda::evaluate("return 21_u*-2_n;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 21_u*-2_b;", state).type() == Nda::Undefined);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_mul_Byte()
{
    NdaState state;

    // Byte*Natural
    QVERIFY(NeoAda::evaluate("return 21_b*2_n;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 21_b*2_n;", state).toInt64() == 42);

    // Byte*Number
    QVERIFY(NeoAda::evaluate("return 21_b*2_d;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 21_b*2_d;", state).toInt64() == 42);

    // Byte*Supernatural
    QVERIFY(NeoAda::evaluate("return 21_b*2_u;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 21_b*2_u;", state).toInt64() == 42);

    // Byte*Byte
    QVERIFY(NeoAda::evaluate("return 21_b*2_b;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 21_b*2_b;", state).toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_pow()
{
    NdaState state;

    QVERIFY(NeoAda::evaluate("return 256**0;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 256**0;", state).toInt64() == 1);
    QVERIFY(NeoAda::evaluate("return 256**3;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 256**3;", state).toInt64() == 16777216);
    QVERIFY(NeoAda::evaluate("return 2_d**-1_n;", state).type() == Nda::Number);
    QVERIFY(NeoAda::evaluate("return 2_d**-1_n;", state).toDouble() == 0.5);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_mod_Number()
{
    NdaState state;

    // Number mod Number
    QVERIFY(NeoAda::evaluate("return 21_d mod 2_d;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 21_d mod 2_d;", state).toInt64() == 0);

    // Number mod Natural
    QVERIFY(NeoAda::evaluate("return 21_d mod 2_n;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 21_d mod 2_n;", state).toInt64() == 0);

    // Number mod Supernatural
    QVERIFY(NeoAda::evaluate("return 21_d mod 2_u;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 21_d mod 2_u;", state).toInt64() == 0);

    // Number mod Byte
    QVERIFY(NeoAda::evaluate("return 21_d mod 2_b;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 21_d mod 2_b;", state).toInt64() == 0);
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_mod_Natural()
{
    NdaState state;

    // Natural % Natural
    QVERIFY(NeoAda::evaluate("return 21_n mod 2_n;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 21_n mod 2_n;", state).toInt64() == 1);

    // Natural % Number
    QVERIFY(NeoAda::evaluate("return 21_n mod 2_d;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 21_n mod 2_d;", state).toInt64() == 0);

    // Natural % Supernatural
    QVERIFY(NeoAda::evaluate("return 21_n mod 2_u;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 21_n mod 2_u;", state).toInt64() == 1);

    // Natural*Byte
    QVERIFY(NeoAda::evaluate("return 21_n mod 2_b;", state).type() == Nda::Natural);
    QVERIFY(NeoAda::evaluate("return 21_n mod 2_b;", state).toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_mod_Supernatural()
{
    NdaState state;

    // Supernatural % Natural
    QVERIFY(NeoAda::evaluate("return 21_u mod 2_n;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 21_u mod 2_n;", state).toInt64() == 1);

    // Supernatural % Number
    QVERIFY(NeoAda::evaluate("return 21_u mod 2_d;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 21_u mod 2_d;", state).toInt64() == 0);

    // Supernatural % Supernatural
    QVERIFY(NeoAda::evaluate("return 21_u mod 2_u;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 21_u mod 2_u;", state).toInt64() == 1);

    // Supernatural % Byte
    QVERIFY(NeoAda::evaluate("return 21_u mod 2_b;", state).type() == Nda::Supernatural);
    QVERIFY(NeoAda::evaluate("return 21_u mod 2_b;", state).toInt64() == 1);

    // unsigned * negative signed
    QVERIFY(NeoAda::evaluate("return 21_u mod -2_n;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 21_u mod -2_b;", state).type() == Nda::Undefined);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_evaluate_mod_Byte()
{
    NdaState state;

    // Byte % Natural
    QVERIFY(NeoAda::evaluate("return 21_b mod 2_n;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 21_b mod 2_n;", state).toInt64() == 1);

    // Byte % Number
    QVERIFY(NeoAda::evaluate("return 21_b mod 2_d;", state).type() == Nda::Undefined);
    QVERIFY(NeoAda::evaluate("return 21_b mod 2_d;", state).toInt64() == 0);

    // Byte % Supernatural
    QVERIFY(NeoAda::evaluate("return 21_b mod 2_u;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 21_b mod 2_u;", state).toInt64() == 1);

    // Byte % Byte
    QVERIFY(NeoAda::evaluate("return 21_b mod 2_b;", state).type() == Nda::Byte);
    QVERIFY(NeoAda::evaluate("return 21_b mod 2_b;", state).toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_MultiInclude()
{
    std::string script = R"(

    with Ada.List;
    with Ada.List;

    declare xs : List := [1,2,3];
    return xs.length();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 3);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_Length()
{
    std::string script = R"(

    with Ada.List;

    declare x : List := [1,2,3];

    return x.length();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 3);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_Append()
{
    std::string script = R"(

    with Ada.List;

    declare x : List := [1,2,3];

    x.append(42);

    return x.length();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 4);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_Insert()
{
    std::string script = R"(

    with Ada.List;

    declare x : List := [1,2,3];

    x.insert(0,42);

    return x[0];
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_Concat()
{
    std::string script = R"(

    with Ada.List;

    declare x : List := [1,2,3];

    x.concat([4,5,6]);

    return x.length();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 6);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_Contains()
{
    std::string script = R"(

    with Ada.List;

    declare x : List := [1,42,3];

    return x.contains(42);
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toBool() == true);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_IndexOf()
{
    std::string script = R"(

    with Ada.List;

    declare x : List := [1,42,3];

    return x.indexOf(42);
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_Flip()
{
    std::string script = R"(

    with Ada.List;

    declare x : List := [40,41,42];
    x.flip();

    return x[0];
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_Flipped()
{
    std::string script = R"(

    with Ada.List;

    declare x : List := [40,41,42];
    declare y : List := x.flipped();

    return y[0];
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_Clear()
{
    std::string script = R"(

    with Ada.List;

    declare x : List := [40,41,42];

    x.clear();

    return #x;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 0);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_AdaList_Clear_Append()
{
    std::string script = R"(

    with Ada.List;

    declare x : List := [40,41,42];

    x.clear();

    x.append(42);

    return x[0];
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_invoke_Fnc1()
{
    std::string script = R"(


    function Hello() return String is
    begin
        return "Hello World";
    end;


    )";

    NdaRuntime r;
    r.runScript(script);

    QVERIFY(r.invokeFnc("Hello").toString() == "Hello World");
}


//-------------------------------------------------------------------------------------------------
void TstParser::test_api_runtime_invoke_Fnc2()
{
    std::string script = R"(


    function Add(a : Natural; b : Natural) return Natural is
    begin
        return a + b;
    end;


    )";

    NdaRuntime r;
    r.runScript(script);

    QVERIFY(r.invokeFnc("Add",NdaValue((int64_t)23),NdaValue((int64_t)42)).toInt64() == (23+42));
}

//-------------------------------------------------------------------------------------------------
//                                       ERROR HANDLING
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_lexer_invalidCharacter()
{
    NdaLexer lexer;
    NdaException ex;

    lexer.setScript("$");
    try {
        while (lexer.nextToken());
    } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidCharacter);
    QVERIFY(ex.line()   == 1);
    QVERIFY(ex.column() == 1);

    lexer.setScript("declare \n $");
    try {
        while (lexer.nextToken());
    } catch (NdaException &e) {
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
    NdaLexer lexer;
    NdaException ex;

    lexer.setScript("\"23456789");
    try {
        while (lexer.nextToken());
    } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);
    QVERIFY(ex.line()   == 1);
    QVERIFY(ex.column() == 9);
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_lexer_invalidBasedLiteral()
{
    NdaLexer lexer;
    NdaException ex;

    //               2#1000_0100#
    lexer.setScript("2#1000_0100 ");
    try {
        while (lexer.nextToken());
    } catch (NdaException &e) {
        ex = e;
    }
    // std::cout << ex.what() << std::endl;
    QVERIFY(ex.code()   == Nada::Error::InvalidBasedLiteral);
    QVERIFY(ex.line()   == 1);
    QVERIFY(ex.column() == 12);

    lexer.setScript("5%5 ");
    try {
        while (lexer.nextToken());
    } catch (NdaException &e) {
        ex = e;
    }
    // std::cout << ex.what() << std::endl;
    QVERIFY(ex.code()   == Nada::Error::InvalidCharacter);

    lexer.setScript("16#FFG ");
    try {
        while (lexer.nextToken());
    } catch (NdaException &e) {
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    NdaException ex;

    try {
        parser.parse(script);
    } catch (NdaException &e) {
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    NdaException ex;

    try {
        parser.parse(script);
    } catch (NdaException &e) {
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    NdaException ex;

    try {
        parser.parse(script);
    } catch (NdaException &e) {
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    NdaException ex;

    try {
        parser.parse(script);
    } catch (NdaException &e) {
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    NdaException ex;

    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }

    QVERIFY(ex.code()   == Nada::Error::KeywordExpected);

    script = "if x then";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then y";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidToken);

    script = "if x then y()";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then y() end";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidToken);

    script = "if x then y(); end";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then y(); end if";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then y(); end if;";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
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

    NdaLexer lexer;
    NdaParser parser(lexer);
    NdaException ex;

    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then f(); else y";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidToken);

    script = "if x then f(); else y()";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);


    script = "if x then f(); else y() end";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::InvalidToken);

    script = "if x then f(); else y(); end";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then f(); else y(); end;";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::KeywordExpected);

    script = "if x then f(); else y(); end while;";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::KeywordExpected);

    script = "if x then f(); else y(); end if";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
        ex = e;
    }
    QVERIFY(ex.code()   == Nada::Error::UnexpectedEof);

    script = "if x then f(); else y(); end if;";
    ex = NdaException();
    try { parser.parse(script); } catch (NdaException &e) {
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
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::NoError);
        QVERIFY(state.unhandledException() == "programerror");
    }

    {
        std::string script = R"(

    declare x : string;
    x := 42;

    )";

        NeoAda::Exception ex;
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::NoError);
        QVERIFY(state.unhandledException() == "programerror");
    }

    {
        std::string script = R"(

    declare x : string := true;

    )";

        NeoAda::Exception ex;
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::NoError);
        QVERIFY(state.unhandledException() == "programerror");
    }

    {
        std::string script = R"(

    declare x : string := 1.23;

    )";

        NeoAda::Exception ex;
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::NoError);
        QVERIFY(state.unhandledException() == "programerror");
    }
}

void TstParser::test_error_interpreter_boolAssignment()
{
    {
        std::string script = R"(

    declare x : boolean := 42;

    )";

        NeoAda::Exception ex;
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::NoError);
        QVERIFY(state.unhandledException() == "programerror");
    }

    {
        std::string script = R"(

    declare x : boolean := "true";

    )";

        NeoAda::Exception ex;
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::NoError);
        QVERIFY(state.unhandledException() == "programerror");
    }
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_runtime_symbolLookup()
{
    {
        std::string script = R"(

            unknownFunctionCall("42");

    )";

        NeoAda::Exception ex;
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::UnknownFunctionCall);
    }
}

//-------------------------------------------------------------------------------------------------
void TstParser::test_error_runtime_divisionByZero()
{
    {
        std::string script = R"(

            declare n : Natural := 42;
            declare x : Natural := n / 0;

    )";

        NeoAda::Exception ex;
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::NoError);
        QVERIFY(state.unhandledException() == "constrainterror");
    }

    {
        std::string script = R"(

            declare n : Supernatural := 42;
            declare x : Supernatural := n / 0;

    )";

        NeoAda::Exception ex;
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::NoError);
        QVERIFY(state.unhandledException() == "constrainterror");
    }

    {
        std::string script = R"(

            declare n : Number := 42;
            declare x : Number := n / 0;

    )";

        NeoAda::Exception ex;
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::NoError);
        QVERIFY(state.unhandledException() == "constrainterror");
    }

    {
        std::string script = R"(

            declare n : Any := 42 / 0;

    )";

        NeoAda::Exception ex;
        NdaState state;
        NeoAda::evaluate(script, state, &ex);
        QVERIFY(ex.code() == Nada::Error::NoError);
        QVERIFY(state.unhandledException() == "constrainterror");
    }
}


extern int runAdaStringTests(int argc, char **argv);
extern int runAdaMathTests(int argc, char **argv);
extern int runAdaTextEncodingTests(int argc, char **argv);
extern int runAdaIoFileTests(int argc, char **argv);
extern int runAdaDateTimeTests(int argc, char **argv);
extern int runAdaRegexpTests(int argc, char **argv);
extern int runAdaJsonTests(int argc, char **argv);

static bool hasRequestedTest(const QMetaObject *metaObject, int argc, char **argv)
{
    bool hasFilter = false;

    for (int i = 1; i < argc; ++i) {
        QString name = QString::fromLocal8Bit(argv[i]);
        if (!name.startsWith(QStringLiteral("test_")))
            continue;

        hasFilter = true;
        name = name.section(':', 0, 0);
        const QByteArray signature = name.toLocal8Bit() + "()";
        if (metaObject->indexOfSlot(signature.constData()) >= 0)
            return true;
    }

    return !hasFilter;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    int status = 0;

    TstParser parserTests;
    if (hasRequestedTest(parserTests.metaObject(), argc, argv))
        status |= QTest::qExec(&parserTests, argc, argv);

    status |= runAdaStringTests(argc, argv);
    status |= runAdaMathTests(argc, argv);
    status |= runAdaTextEncodingTests(argc, argv);
    status |= runAdaIoFileTests(argc, argv);
    status |= runAdaDateTimeTests(argc, argv);
    status |= runAdaRegexpTests(argc, argv);
    status |= runAdaJsonTests(argc, argv);

    std::cout << "********* Finished testing *********" << std::endl;
    std::cout << "Status: " << status << " " << (status == 0 ? "(OK)":"(ERROR)") << std::endl;
    return status;
}

#include "tst_parser.moc"

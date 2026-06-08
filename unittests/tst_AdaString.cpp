#include <QtTest>
#include <QString>

#include <libneoada/runtime.h>
#include <libneoada/state.h>

class TstAdaString : public QObject
{
    Q_OBJECT

private slots:
    void test_api_runtime_AdaString_Length();
    void test_api_runtime_AdaString_Append();
    void test_api_runtime_AdaString_Insert();
    void test_api_runtime_AdaString_ToUppper();
    void test_api_runtime_AdaString_ToLower();
    void test_api_runtime_AdaString_Uppper();
    void test_api_runtime_AdaString_Lower();
    void test_api_runtime_AdaString_Contains();
    void test_api_runtime_AdaString_IndexOf();
    void test_api_runtime_AdaString_Trim();
    void test_api_runtime_AdaString_Trimmed();
    void test_api_runtime_AdaString_Chop1();
    void test_api_runtime_AdaString_Chop2();
    void test_api_runtime_AdaString_Chopped();
    void test_api_runtime_AdaString_Slice();
    void test_api_runtime_AdaString_Sliced();
    void test_api_runtime_AdaString_FromBytes();
    void test_api_runtime_AdaString_ToBytes();
    void test_api_runtime_AdaString_FormatFloating();
    void test_api_runtime_AdaString_FormatInteger();
    void test_api_runtime_AdaString_FormatConstraintError();
    void test_api_runtime_AdaString_ToNumber();
    void test_api_runtime_AdaString_ToNatural();
    void test_api_runtime_AdaString_ToBool();
    void test_api_runtime_AdaString_IsConversions();
    void test_api_runtime_AdaString_ConversionProgramError();
};

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Length()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "123456";

    return x.length();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 6);
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Append()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "123456";

    x.append("789");

    return x;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "123456789");
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Insert()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "123789";

    x.insert(3,"456");

    return x;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "123456789");

}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_ToUppper()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda";

    return x.toUpper();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "NEOADA");
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_ToLower()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda";

    return x.toLower();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "neoada");
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Uppper()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda";

    x.upper();

    return x;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "NEOADA");
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Lower()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda";

    x.lower();

    return x;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "neoada");
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Contains()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda";

    return x.contains("Ada");
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toBool() == true);
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_IndexOf()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda";

    return x.indexOf("Ada");
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 3);
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Trim()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "  NeoAda  ";

    x.trim();

    return x;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "NeoAda");
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Trimmed()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "  NeoAda  ";

    return x.trimmed();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "NeoAda");
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Chop1()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda  ";

    x.chop(2);

    return x;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "NeoAda");
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Chop2()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda  ";

    x.chop(27);

    return x;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "");
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Chopped()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda  ";

    return x.chopped(2);
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "NeoAda");
}


//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Slice()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda";

    x.slice(0,3);

    return x;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "Neo");
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_Sliced()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "NeoAda";

    return x.sliced(0,3);
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "Neo");
}


//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_FromBytes()
{
    std::string script = R"(

    with Ada.Bytes;
    with Ada.String;

    declare raw : Bytes;
    raw.append(72_b);
    raw.append(228_b);
    declare text : String := String:fromBytes(raw, "latin1");
    declare encoded : Bytes := text.toBytes("utf-8");

    if encoded.length() = 3 and encoded[0] = 72_b and encoded[1] = 195_b and encoded[2] = 164_b then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_ToBytes()
{
    std::string script = R"(

    with Ada.Bytes;
    with Ada.String;

    declare text : String := "Az";
    declare encoded : Bytes := text.toBytes("utf-16");

    if encoded.length() = 6 and encoded[0] = 255_b and encoded[1] = 254_b and encoded[2] = 65_b and encoded[3] = 0_b and encoded[4] = 122_b and encoded[5] = 0_b then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}


//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_FormatFloating()
{
    const std::string script = R"(
    with Ada.String;

    if String:format(12.3456, "f2") <> "12.35" then
        return 0;
    end if;
    if String:format(12.3456, "e2") <> "1.23e+01" then
        return 0;
    end if;
    if String:format(12.3456, "E2") <> "1.23E+01" then
        return 0;
    end if;
    if String:format(12.3456, "g4") <> "12.35" then
        return 0;
    end if;
    if String:format(2.5, "f") <> "2.500000" then
        return 0;
    end if;
    return 1;
    )";

    NdaRuntime runtime;
    const auto ret = runtime.runScript(script);
    QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
    QCOMPARE(ret.toInt64(), int64_t(1));
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_FormatInteger()
{
    const std::string script = R"(
    with Ada.String;

    if String:format(42, "d5") <> "00042" then
        return 0;
    end if;
    if String:format(255, "x4") <> "00ff" then
        return 0;
    end if;
    if String:format(255, "b") <> "11111111" then
        return 0;
    end if;
    if String:format(255, "o") <> "377" then
        return 0;
    end if;
    return 1;
    )";

    NdaRuntime runtime;
    const auto ret = runtime.runScript(script);
    QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
    QCOMPARE(ret.toInt64(), int64_t(1));
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_FormatConstraintError()
{
    const std::string script = R"(
    with Ada.String;

    begin
        String:format(12.5, "x");
        return 0;
    exception
        when ConstraintError => return 1;
    end;
    )";

    NdaRuntime runtime;
    const auto ret = runtime.runScript(script);
    QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
    QCOMPARE(ret.toInt64(), int64_t(1));
    QVERIFY(runtime.state()->unhandledException().empty());
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_ToNumber()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "  12.5 ";
    return x.toNumber();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toDouble() == 12.5);
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_ToNatural()
{
    std::string script = R"(

    with Ada.String;

    declare x : String := "  42 ";
    return x.toNatural();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 42);
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_ToBool()
{
    std::string script = R"(

    with Ada.String;

    declare t : String := "true";
    declare f : String := " FALSE ";
    if t.toBool() = true and f.toBool() = false then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_IsConversions()
{
    std::string script = R"(

    with Ada.String;

    declare numberOk : String := "-12.5e2";
    declare naturalOk : String := "42";
    declare boolOk : String := "false";
    declare bad : String := "12abc";

    if numberOk.isNumber() = true and naturalOk.isNatural() = true and boolOk.isBool() = true
       and bad.isNumber() = false and bad.isNatural() = false and bad.isBool() = false then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaString::test_api_runtime_AdaString_ConversionProgramError()
{
    std::string script = R"(

    with Ada.String;

    function Test() return Natural is
        x : String := "not-a-number";
    begin
        x.toNatural();
        return 0;
    exception
        when ProgramError => return 1;
    end;

    return Test();
    )";

    NdaRuntime r;

    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
    QVERIFY(r.state()->unhandledException().empty());
}


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

int runAdaStringTests(int argc, char **argv)
{
    TstAdaString tests;
    if (!hasRequestedTest(tests.metaObject(), argc, argv))
        return 0;

    return QTest::qExec(&tests, argc, argv);
}

#include "tst_AdaString.moc"

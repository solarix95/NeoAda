#include <QtTest>
#include <QString>

#include <libneoada/runtime.h>

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

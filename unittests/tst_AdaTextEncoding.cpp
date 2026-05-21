#include <QtTest>
#include <QString>

#include <libneoada/runtime.h>

class TstAdaTextEncoding : public QObject
{
    Q_OBJECT

private slots:
    void test_api_runtime_AdaTextEncoding_Utf8Roundtrip();
    void test_api_runtime_AdaTextEncoding_Utf16Encode();
    void test_api_runtime_AdaTextEncoding_Utf16DecodeLittleEndian();
    void test_api_runtime_AdaTextEncoding_Utf16DecodeBigEndianBom();
    void test_api_runtime_AdaTextEncoding_Latin1Roundtrip();
    void test_api_runtime_AdaTextEncoding_AsciiEncode();
    void test_api_runtime_AdaTextEncoding_UnknownEncoding();
};

//-------------------------------------------------------------------------------------------------
void TstAdaTextEncoding::test_api_runtime_AdaTextEncoding_Utf8Roundtrip()
{
    std::string script = R"(
    with Ada.Bytes;
    with Ada.Text.Encoding;

    declare raw : Bytes;
    raw.append(195_b);
    raw.append(164_b);
    declare text : String := Encoding:decode(raw, "utf-8");
    declare encoded : Bytes := Encoding:encode(text, "utf-8");
    if encoded.length() = 2 and encoded[0] = 195_b and encoded[1] = 164_b then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaTextEncoding::test_api_runtime_AdaTextEncoding_Utf16Encode()
{
    std::string script = R"(
    with Ada.Bytes;
    with Ada.Text.Encoding;

    declare encoded : Bytes := Encoding:encode("Az", "utf-16");
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
void TstAdaTextEncoding::test_api_runtime_AdaTextEncoding_Utf16DecodeLittleEndian()
{
    std::string script = R"(
    with Ada.Bytes;
    with Ada.Text.Encoding;

    declare raw : Bytes;
    raw.append(65_b);
    raw.append(0_b);
    raw.append(122_b);
    raw.append(0_b);
    declare text : String := Encoding:decode(raw, "utf16");
    declare encoded : Bytes := Encoding:encode(text, "ascii");
    if encoded.length() = 2 and encoded[0] = 65_b and encoded[1] = 122_b then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaTextEncoding::test_api_runtime_AdaTextEncoding_Utf16DecodeBigEndianBom()
{
    std::string script = R"(
    with Ada.Bytes;
    with Ada.Text.Encoding;

    declare raw : Bytes;
    raw.append(254_b);
    raw.append(255_b);
    raw.append(0_b);
    raw.append(65_b);
    raw.append(0_b);
    raw.append(122_b);
    declare text : String := Encoding:decode(raw, "UTF-16");
    declare encoded : Bytes := Encoding:encode(text, "ascii");
    if encoded.length() = 2 and encoded[0] = 65_b and encoded[1] = 122_b then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaTextEncoding::test_api_runtime_AdaTextEncoding_Latin1Roundtrip()
{
    std::string script = R"(
    with Ada.Bytes;
    with Ada.Text.Encoding;

    declare raw : Bytes;
    raw.append(72_b);
    raw.append(228_b);
    declare text : String := Encoding:decode(raw, "latin1");
    declare encoded : Bytes := Encoding:encode(text, "latin1");
    if encoded.length() = 2 and encoded[0] = 72_b and encoded[1] = 228_b then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaTextEncoding::test_api_runtime_AdaTextEncoding_AsciiEncode()
{
    std::string script = R"(
    with Ada.Bytes;
    with Ada.Text.Encoding;

    declare encoded : Bytes := Encoding:encode("Neo", "ascii");
    if encoded.length() = 3 and encoded[0] = 78_b and encoded[1] = 101_b and encoded[2] = 111_b then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaTextEncoding::test_api_runtime_AdaTextEncoding_UnknownEncoding()
{
    std::string script = R"(
    with Ada.Text.Encoding;

    return Encoding:encode("Neo", "unknown-encoding");
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.type() == Nda::Undefined);
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

int runAdaTextEncodingTests(int argc, char **argv)
{
    TstAdaTextEncoding tests;
    if (!hasRequestedTest(tests.metaObject(), argc, argv))
        return 0;

    return QTest::qExec(&tests, argc, argv);
}

#include "tst_AdaTextEncoding.moc"

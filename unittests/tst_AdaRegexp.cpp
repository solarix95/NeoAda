#include <QtTest>
#include <QString>

#include <libneoada/runtime.h>

class TstAdaRegexp : public QObject
{
    Q_OBJECT

private slots:
    void test_api_runtime_AdaRegexp_Basic();
    void test_api_runtime_AdaRegexp_CapturesReplaceSplit();
};

//-------------------------------------------------------------------------------------------------
void TstAdaRegexp::test_api_runtime_AdaRegexp_Basic()
{
    std::string script = R"NEOADA(
    with Ada.Regexp;

    if Regexp:match("abc123", "[a-z]+[0-9]+") and Regexp:contains("abc123", "[0-9]+") and Regexp:firstMatch("abc123def", "[0-9]+") = "123" and Regexp:match("abc123", "[0-9]+") = false then
        return 1;
    end if;
    return 0;
    )NEOADA";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaRegexp::test_api_runtime_AdaRegexp_CapturesReplaceSplit()
{
    std::string script = R"NEOADA(
    with Ada.Regexp;

    declare caps : List := Regexp:captures("user:42", "([a-z]+):([0-9]+)");
    declare parts : List := Regexp:split("a,b,,c", ",");
    declare replaced : String := Regexp:replace("a1 b22 c333", "[0-9]+", "X");

    if #caps = 3 and caps[0] = "user:42" and caps[1] = "user" and caps[2] = "42" and #parts = 4 and parts[0] = "a" and parts[2] = "" and replaced = "aX bX cX" then
        return 1;
    end if;
    return 0;
    )NEOADA";

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

int runAdaRegexpTests(int argc, char **argv)
{
    TstAdaRegexp tests;
    if (!hasRequestedTest(tests.metaObject(), argc, argv))
        return 0;

    return QTest::qExec(&tests, argc, argv);
}

#include "tst_AdaRegexp.moc"

#include <QtTest>
#include <QString>

#include <libneoada/runtime.h>

class TstAdaDict : public QObject
{
    Q_OBJECT

private slots:
    void test_api_runtime_AdaDict_Basic();
    void test_api_runtime_AdaDict_KeysValues();
    void test_api_runtime_AdaDict_ValueDefault();
};

void TstAdaDict::test_api_runtime_AdaDict_Basic()
{
    const std::string script = R"(
    with Ada.Dict;

    declare data : Dict := {"name":"Ada", "age":12};
    if data.length() <> 2 or data.contains("name") = false or data.contains("missing") then
        return 0;
    end if;
    if data.remove("name") <> 1 or data.remove("name") <> 0 or data.length() <> 1 then
        return 0;
    end if;
    data.clear();
    return data.length();
    )";

    NdaRuntime runtime;
    const auto ret = runtime.runScript(script);
    QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
    QCOMPARE(ret.toInt64(), int64_t(0));
}

void TstAdaDict::test_api_runtime_AdaDict_KeysValues()
{
    const std::string script = R"(
    with Ada.Dict;

    declare data : Dict := {"b":20, "a":10};
    declare keys : List := data.keys();
    declare values : List := data.values();
    if #keys <> 2 or #values <> 2 then
        return false;
    end if;
    return keys[0] = "a" and keys[1] = "b" and values[0] = 10 and values[1] = 20;
    )";

    NdaRuntime runtime;
    const auto ret = runtime.runScript(script);
    QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
    QVERIFY(ret.toBool());
}

void TstAdaDict::test_api_runtime_AdaDict_ValueDefault()
{
    const std::string script = R"(
    with Ada.Dict;

    declare data : Dict := {"answer":42};
    declare found : Natural := data.value("answer", 0);
    declare fallback : String := data.value("missing", "unknown");
    return found = 42 and fallback = "unknown" and data.length() = 1;
    )";

    NdaRuntime runtime;
    const auto ret = runtime.runScript(script);
    QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
    QVERIFY(ret.toBool());
}

static bool hasRequestedTest(const QMetaObject *metaObject, int argc, char **argv)
{
    bool hasFilter = false;
    for (int i = 1; i < argc; ++i) {
        QString name = QString::fromLocal8Bit(argv[i]);
        if (!name.startsWith(QStringLiteral("test_")))
            continue;
        hasFilter = true;
        name = name.section(":", 0, 0);
        const QByteArray signature = name.toLocal8Bit() + "()";
        if (metaObject->indexOfSlot(signature.constData()) >= 0)
            return true;
    }
    return !hasFilter;
}

int runAdaDictTests(int argc, char **argv)
{
    TstAdaDict tests;
    if (!hasRequestedTest(tests.metaObject(), argc, argv))
        return 0;
    return QTest::qExec(&tests, argc, argv);
}

#include "tst_AdaDict.moc"

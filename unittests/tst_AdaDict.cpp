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
    void test_api_runtime_AdaDict_IsEmptyItems();
    void test_api_runtime_AdaDict_Ensure();
    void test_api_runtime_AdaDict_Take();
    void test_api_runtime_AdaDict_TakeConstraintError();
    void test_api_runtime_AdaDict_UpdateUpdated();
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


void TstAdaDict::test_api_runtime_AdaDict_IsEmptyItems()
{
    const std::string script = R"(
    with Ada.Dict;

    declare data : Dict := {};
    if data.isEmpty() = false then
        return false;
    end if;

    data{"b"} := 20;
    data{"a"} := 10;

    declare items : List := data.items();
    return data.isEmpty() = false
        and #items = 2
        and items[0]{"key"} = "a"
        and items[0]{"value"} = 10
        and items[1]{"key"} = "b"
        and items[1]{"value"} = 20;
    )";

    NdaRuntime runtime;
    const auto ret = runtime.runScript(script);
    QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
    QVERIFY(ret.toBool());
}

void TstAdaDict::test_api_runtime_AdaDict_Ensure()
{
    const std::string script = R"(
    with Ada.Dict;

    declare data : Dict := {"count":5};
    declare found : Natural := data.ensure("count", 0);
    declare inserted : String := data.ensure("name", "NeoAda");

    return found = 5
        and inserted = "NeoAda"
        and data{"count"} = 5
        and data{"name"} = "NeoAda"
        and data.length() = 2;
    )";

    NdaRuntime runtime;
    const auto ret = runtime.runScript(script);
    QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
    QVERIFY(ret.toBool());
}

void TstAdaDict::test_api_runtime_AdaDict_Take()
{
    {
        const std::string script = R"(
        with Ada.Dict;
        declare data : Dict := {"x":42};
        return data.take("x");
        )";

        NdaRuntime runtime;
        const auto ret = runtime.runScript(script);
        QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
        QCOMPARE(ret.toInt64(), int64_t(42));
    }

    {
        const std::string script = R"(
        with Ada.Dict;
        declare data : Dict := {"x":42, "y":23};
        declare x : Natural := data.take("x");
        return data.contains("x") = false and data.contains("y") and data.length() = 1;
        )";

        NdaRuntime runtime;
        const auto ret = runtime.runScript(script);
        QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
        QVERIFY(ret.toBool());
    }

    {
        const std::string script = R"(
        with Ada.Dict;
        declare data : Dict := {};
        return data.take("missing", "fallback");
        )";

        NdaRuntime runtime;
        const auto ret = runtime.runScript(script);
        QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
        QCOMPARE(ret.toString(), std::string("fallback"));
    }
}

void TstAdaDict::test_api_runtime_AdaDict_TakeConstraintError()
{
    const std::string script = R"(
    with Ada.Dict;

    declare data : Dict := {};
    begin
        data.take("missing");
    exception
        when ConstraintError => return 1;
    end;
    return 0;
    )";

    NdaRuntime runtime;
    const auto ret = runtime.runScript(script);
    QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
    QCOMPARE(ret.toInt64(), int64_t(1));
}

void TstAdaDict::test_api_runtime_AdaDict_UpdateUpdated()
{
    const std::string script = R"(
    with Ada.Dict;

    declare a : Dict := {"x":1, "keep":7};
    declare b : Dict := {"x":2, "y":3};
    declare c : Dict := a.updated(b);

    declare mask : Natural := 0;
    if a{"x"} = 1 then
        mask := mask + 1;
    end if;
    if a.contains("y") = false then
        mask := mask + 2;
    end if;
    if c{"x"} = 2 then
        mask := mask + 4;
    end if;
    if c{"y"} = 3 then
        mask := mask + 8;
    end if;
    if c{"keep"} = 7 then
        mask := mask + 16;
    end if;

    a.update(b);
    if a{"x"} = 2 then
        mask := mask + 32;
    end if;
    if a{"y"} = 3 then
        mask := mask + 64;
    end if;
    if a{"keep"} = 7 then
        mask := mask + 128;
    end if;
    return mask;
    )";

    NdaRuntime runtime;
    const auto ret = runtime.runScript(script);
    QVERIFY2(!runtime.hasError(), runtime.lastError().c_str());
    QCOMPARE(ret.toInt64(), int64_t(255));
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

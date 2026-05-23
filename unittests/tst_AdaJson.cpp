#include <QtTest>
#include <QString>

#include <libneoada/runtime.h>

class TstAdaJson : public QObject
{
    Q_OBJECT

private slots:
    void test_api_runtime_AdaJson_StringifyParse();
    void test_api_runtime_AdaJson_TextFile();
};

//-------------------------------------------------------------------------------------------------
void TstAdaJson::test_api_runtime_AdaJson_StringifyParse()
{
    std::string script = R"NEOADA(
    with Ada.Json;

    declare data   : Dict   := {"name":"Ada", "age":12, "active":true, "skills":["NeoAda", "Qt"], "score":2.5};
    declare text   : String := Json:toString(data);
    declare parsed : Dict   := Json:fromString(text);

    if parsed{"name"} = "Ada" and parsed{"age"} = 12 and parsed{"active"} and parsed{"skills"}[1] = "Qt" and parsed{"score"} = 2.5 then
        return 1;
    end if;
    return 0;
    )NEOADA";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaJson::test_api_runtime_AdaJson_TextFile()
{
    std::string script = R"NEOADA(
    with Ada.Json;
    with Ada.Io.File;

    declare data : Dict := {"title":"NeoAda", "numbers":[1,2,3]};
    declare fileOut : TextFile := TextFile:create("/tmp/neoada_json_unittest.json");
    Json:write(fileOut, data);
    fileOut.close();

    declare input : TextFile := TextFile:openRead("/tmp/neoada_json_unittest.json");
    declare parsed : Dict := Json:read(input);
    input.close();

    if parsed{"title"} = "NeoAda" and parsed{"numbers"}[2] = 3 then
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

int runAdaJsonTests(int argc, char **argv)
{
    TstAdaJson tests;
    if (!hasRequestedTest(tests.metaObject(), argc, argv))
        return 0;

    return QTest::qExec(&tests, argc, argv);
}

#include "tst_AdaJson.moc"

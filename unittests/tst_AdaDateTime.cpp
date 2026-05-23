#include <QtTest>
#include <QString>

#include <libneoada/runtime.h>

class TstAdaDateTime : public QObject
{
    Q_OBJECT

private slots:
    void test_api_runtime_AdaDateTime_Date();
    void test_api_runtime_AdaDateTime_Time();
    void test_api_runtime_AdaDateTime_DateTime();
    void test_api_runtime_AdaDateTime_ChainedMethods();
    void test_api_runtime_AdaDateTime_SetDateTimeSecsTo();
    void test_api_runtime_AdaDateTime_Now();
};

//-------------------------------------------------------------------------------------------------
void TstAdaDateTime::test_api_runtime_AdaDateTime_Date()
{
    std::string script = R"(
    with Ada.DateTime;

    declare d : Date := Date:fromString("2026-05-21", "yyyy-MM-dd");
    declare next : Date := d.addDays(10);
    declare prev : Date := d.addDays(0 - 1);

    if d.toString("dd.MM.yyyy") = "21.05.2026" and next.toString("yyyy-MM-dd") = "2026-05-31" and prev.toString("yyyy-MM-dd") = "2026-05-20" then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaDateTime::test_api_runtime_AdaDateTime_Time()
{
    std::string script = R"(
    with Ada.DateTime;

    declare t : Time := Time:fromString("23:59:50", "HH:mm:ss");
    declare plus : Time := t.addSecs(15);
    declare minus : Time := t.addSecs(0 - 60);

    if t.toString("HH-mm-ss") = "23-59-50" and plus.toString("HH:mm:ss") = "00:00:05" and minus.toString("HH:mm:ss") = "23:58:50" then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaDateTime::test_api_runtime_AdaDateTime_DateTime()
{
    std::string script = R"(
    with Ada.DateTime;

    declare dt : DateTime := DateTime:fromString("2026-12-31 23:59:50", "yyyy-MM-dd HH:mm:ss");
    declare plus : DateTime := dt.addSecs(15);
    declare future : DateTime := dt.addDays(2);

    if dt.toString("yyyy/MM/dd HH:mm:ss") = "2026/12/31 23:59:50" and plus.toString("yyyy-MM-dd HH:mm:ss") = "2027-01-01 00:00:05" and future.toString("yyyy-MM-dd") = "2027-01-02" then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}


//-------------------------------------------------------------------------------------------------
void TstAdaDateTime::test_api_runtime_AdaDateTime_ChainedMethods()
{
    std::string script = R"(
    with Ada.DateTime;

    declare start : DateTime := DateTime:fromString("2026-05-21 09:30:00", "yyyy-MM-dd HH:mm:ss");
    declare deadline : DateTime := start.addDays(3).addSecs(2 * 60 * 60);

    if deadline.toString("yyyy-MM-dd HH:mm:ss") = "2026-05-24 11:30:00" and #DateTime:now().toString("yyyy-MM-dd") = 10 then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaDateTime::test_api_runtime_AdaDateTime_SetDateTimeSecsTo()
{
    std::string script = R"(
    with Ada.DateTime;

    declare dt : DateTime := DateTime:fromString("2026-05-21 09:30:00", "yyyy-MM-dd HH:mm:ss");
    declare changed : DateTime := dt.setDate(2026, 5, 22).setTime(10, 31, 5);
    declare d : Date := Date:fromString("2026-06-01", "yyyy-MM-dd");
    declare t : Time := Time:fromString("12:00:30", "HH:mm:ss");
    declare changed2 : DateTime := changed.setDate(d).setTime(t);

    if dt.secsTo(changed) = 90065 and changed2.toString("yyyy-MM-dd HH:mm:ss") = "2026-06-01 12:00:30" and changed.secsTo(dt) = -90065 then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaDateTime::test_api_runtime_AdaDateTime_Now()
{
    std::string script = R"(
    with Ada.DateTime;

    declare d : Date := Date:now();
    declare t : Time := Time:now();
    declare dt : DateTime := DateTime:now();

    if #d.toString("yyyy-MM-dd") = 10 and #t.toString("HH:mm:ss") = 8 and #dt.toString("yyyy-MM-dd HH:mm:ss") = 19 then
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

//-------------------------------------------------------------------------------------------------
int runAdaDateTimeTests(int argc, char **argv)
{
    TstAdaDateTime tests;
    if (!hasRequestedTest(tests.metaObject(), argc, argv))
        return 0;

    return QTest::qExec(&tests, argc, argv);
}

#include "tst_AdaDateTime.moc"

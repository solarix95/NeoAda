#include <QtTest>
#include <QString>

#include <libneoada/runtime.h>

class TstAdaMath : public QObject
{
    Q_OBJECT

private slots:
    void test_api_runtime_AdaMath_Basic();
    void test_api_runtime_AdaMath_RoundingClamp();
    void test_api_runtime_AdaMath_TrigConstants();
    void test_api_runtime_AdaMath_Predicates();
};

//-------------------------------------------------------------------------------------------------
void TstAdaMath::test_api_runtime_AdaMath_Basic()
{
    std::string script = R"(
    with Ada.Math;

    declare eps : Number := 0.000001;
    declare neg : Number := 0.0 - 1.0;
    declare negFive : Number := 0.0 - 5.0;

    if Math:abs(negFive) = 5 and Math:min(3, 7) = 3 and Math:max(3, 7) = 7 and Math:pow(2, 8) = 256 and Math:hypot(3, 4) = 5 and Math:fmod(7, 4) = 3 and Math:copySign(5, neg) = negFive and Math:sign(neg) = neg and Math:abs(Math:sqrt(81) - 9) < eps and Math:abs(Math:cbrt(27) - 3) < eps then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaMath::test_api_runtime_AdaMath_RoundingClamp()
{
    std::string script = R"(
    with Ada.Math;

    if Math:floor(2.9) = 2 and Math:ceil(2.1) = 3 and Math:round(2.6) = 3 and Math:trunc(2.9) = 2 and Math:clamp(12, 0, 10) = 10 and Math:clamp(4, 10, 0) = 4 then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaMath::test_api_runtime_AdaMath_TrigConstants()
{
    std::string script = R"(
    with Ada.Math;

    declare eps : Number := 0.000001;
    declare pi : Number := Math:pi();
    declare e : Number := Math:e();

    if Math:abs(Math:sin(pi / 2) - 1) < eps and Math:abs(Math:cos(0) - 1) < eps and Math:abs(Math:tau() - 2 * pi) < eps and Math:abs(Math:degrees(pi) - 180) < eps and Math:abs(Math:radians(180) - pi) < eps and Math:abs(Math:log(e) - 1) < eps and Math:abs(Math:log10(100) - 2) < eps then
        return 1;
    end if;
    return 0;
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 1);
}

//-------------------------------------------------------------------------------------------------
void TstAdaMath::test_api_runtime_AdaMath_Predicates()
{
    std::string script = R"(
    with Ada.Math;

    if Math:isNan(Math:nan()) and Math:isFinite(42) and Math:isFinite(Math:nan()) = false and Math:isInf(Math:infinity()) then
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

int runAdaMathTests(int argc, char **argv)
{
    TstAdaMath tests;
    if (!hasRequestedTest(tests.metaObject(), argc, argv))
        return 0;

    return QTest::qExec(&tests, argc, argv);
}

#include "tst_AdaMath.moc"

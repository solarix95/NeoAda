#include <QtTest>
#include <QString>

#include <fstream>

#include <libneoada/runtime.h>

class TstAdaIoFile : public QObject
{
    Q_OBJECT

private slots:
    void test_api_runtime_AdaIoFile_FileBytes_CreateReadAll();
    void test_api_runtime_AdaIoFile_TextFile_CreateReadAll();
    void test_api_runtime_AdaIoFile_TextFile_ExistsOpenRead();
    void test_api_runtime_AdaIoFile_TextFile_DictMembers();
};

//-------------------------------------------------------------------------------------------------
void TstAdaIoFile::test_api_runtime_AdaIoFile_FileBytes_CreateReadAll()
{
    std::string script = R"(
    with Ada.Io.File;
    with Ada.Bytes;

    declare bytes : Bytes;
    bytes.append(0_b);
    bytes.append(65_b);
    bytes.append(255_b);

    declare f : File := File:create("/tmp/neoada_io_file_bytes.bin");
    f.write(bytes);
    declare read : Bytes := f.readAll();
    return read.length() * 1000 + read[0] * 100 + read[1] + read[2];
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toInt64() == 3320);
}

//-------------------------------------------------------------------------------------------------
void TstAdaIoFile::test_api_runtime_AdaIoFile_TextFile_CreateReadAll()
{
    std::string script = R"(
    with Ada.Io.File;

    declare f : TextFile := TextFile:create("/tmp/neoada_io_textfile_create.txt");
    f.writeLine("Hello");
    f.write("NeoAda");
    return f.readAll();
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "Hello\nNeoAda");
}

//-------------------------------------------------------------------------------------------------
void TstAdaIoFile::test_api_runtime_AdaIoFile_TextFile_ExistsOpenRead()
{
    {
        std::ofstream f("/tmp/neoada_io_textfile_read.txt");
        f << "Line1\nLine2\n";
    }

    std::string script = R"(
    with Ada.Io.File;

    declare ok : Boolean := TextFile:exists("/tmp/neoada_io_textfile_read.txt");
    declare f : TextFile := TextFile:openRead("/tmp/neoada_io_textfile_read.txt");
    if ok and f.isOpen() then
        return f.readLine();
    end if;
    return "";
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "Line1");
}

//-------------------------------------------------------------------------------------------------
void TstAdaIoFile::test_api_runtime_AdaIoFile_TextFile_DictMembers()
{
    std::string script = R"(
    with Ada.Io.File;

    declare f : TextFile := TextFile:create("/tmp/neoada_io_textfile_members.txt");
    f{"encoding"} := "latin1";
    return f{"__type"} & ":" & f{"mode"} & ":" & f{"encoding"};
    )";

    NdaRuntime r;
    auto ret = r.runScript(script);

    QVERIFY(ret.toString() == "TextFile:create:latin1");
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

int runAdaIoFileTests(int argc, char **argv)
{
    TstAdaIoFile tests;
    if (!hasRequestedTest(tests.metaObject(), argc, argv))
        return 0;

    return QTest::qExec(&tests, argc, argv);
}

#include "tst_AdaIoFile.moc"

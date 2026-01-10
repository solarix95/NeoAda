#include "neoadaedit.h"
#include "ui_neoadaedit.h"
#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>
#include <QSaveFile>
#include <QCloseEvent>
#include <QFontDatabase>
#include <QTextEdit>
#include <QApplication>

#include "neoadahighlighter.h"
#include <state.h>
#include <runtime.h>

static const char* kOrgName  = "Solarix95";
static const char* kAppName  = "NeoAda";
static const char* kGroup    = "NeoAdaEdit";

//-------------------------------------------------------------------------------------------------
NeoAdaEdit::NeoAdaEdit(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NeoAdaEdit)
    , mAda(*(new NdaRuntime()))
{
    ui->setupUi(this);

    connect(ui->btnOpen, &QPushButton::clicked, this, &NeoAdaEdit::onOpen);
    connect(ui->btnSave, &QPushButton::clicked, this, &NeoAdaEdit::onSave);
    connect(ui->btnSaveAs, &QPushButton::clicked, this, &NeoAdaEdit::onSaveAs);

    connect(ui->btnPlay, &QPushButton::clicked, this, &NeoAdaEdit::onPlay);
    connect(ui->btnStop, &QPushButton::clicked, this, &NeoAdaEdit::onStop);

    initEditor();
}

//-------------------------------------------------------------------------------------------------
NeoAdaEdit::~NeoAdaEdit()
{
    delete ui;
    delete &mAda;
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::saveState()
{
    QSettings s(kOrgName, kAppName);
    s.beginGroup(kGroup);

    s.setValue("currentFileName", mCurrentFileName);

    // Optional: Fenstergeometrie, Splitter, etc. (falls vorhanden)
    // s.setValue("geometry", saveGeometry());

    // Optional: Cursorposition / Scroll (minimal sinnvoll)
    if (ui->txtScript) {
        s.setValue("cursorPosition", ui->txtScript->textCursor().position());
        s.setValue("plainText", ui->txtScript->toPlainText()); // optional: last buffer
    }

    s.endGroup();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::restoreState()
{
    QSettings s(kOrgName, kAppName);
    s.beginGroup(kGroup);

    mCurrentFileName = s.value("currentFileName").toString();

    // Optional:
    // restoreGeometry(s.value("geometry").toByteArray());

    // Wenn du den letzten Buffer wiederherstellen willst (ohne Datei):
    const QString lastText = s.value("plainText").toString();
    if (!lastText.isEmpty() && ui->txtScript) {
        ui->txtScript->setPlainText(lastText);
        auto c = ui->txtScript->textCursor();
        c.setPosition(s.value("cursorPosition", 0).toInt());
        ui->txtScript->setTextCursor(c);
    }

    s.endGroup();

    // Wenn ein Dateiname existiert, kannst du automatisch laden:
    if (!mCurrentFileName.isEmpty()) {
        loadCurrentFile();
    }
}

//-------------------------------------------------------------------------------------------------
bool NeoAdaEdit::isDirty() const
{
    if (!ui->txtScript) return false;

    // Wenn nie gespeichert/geladen wurde: dirty wenn Text nicht leer
    if (mCurrentFileName.isEmpty()) {
        return !ui->txtScript->toPlainText().isEmpty();
    }

    QFile f(mCurrentFileName);
    if (!f.exists()) {
        // Datei "weg": dann gilt der Buffer als dirty, sobald Text existiert
        return true;
    }

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Kann nicht lesen -> konservativ: treat as dirty
        return true;
    }

    QTextStream in(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    in.setCodec("UTF-8");
#endif

    const QString fileText = in.readAll();
    const QString editorText = ui->txtScript->toPlainText();

    return fileText != editorText;
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::loadCurrentFile()
{
    if (mCurrentFileName.isEmpty() || !ui->txtScript)
        return;

    QFile f(mCurrentFileName);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this,
                             tr("Öffnen fehlgeschlagen"),
                             tr("Datei konnte nicht geöffnet werden:\n%1")
                                 .arg(mCurrentFileName));
        return;
    }

    QTextStream in(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    in.setCodec("UTF-8");
#endif

    ui->txtScript->setPlainText(in.readAll());

    // Optional: Titel/Status anzeigen
    // setWindowTitle(QFileInfo(mCurrentFileName).fileName());
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::initRuntime()
{
    mAda.reset();
    mAda.state()->bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        ui->txtOutput->appendPlainText(QString::fromStdString(args.at("message").toString()));
        qApp->processEvents();
        return true;
    });
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::initEditor()
{
    // Monospace font
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(11);
    ui->txtScript->setFont(font);

    // Better readability
    ui->txtScript->setLineWrapMode(QTextEdit::NoWrap);

// Tab width (4 spaces)
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    const int spaceWidth = QFontMetrics(font).horizontalAdvance(' ');
#else
    const int spaceWidth = QFontMetrics(font).width(' ');
#endif
    ui->txtScript->setTabStopDistance(spaceWidth * 4);

    // Optional: nice default text
    ui->txtScript->setPlaceholderText(
        "declare x : Number := 42;\n"
        "if x > 0 then\n"
        "    print(\"x=\" & x);\n"
        "end if;\n"
        );

    // Attach syntax highlighter
    mHighlighter = new NeoAdaHighlighter(ui->txtScript->document());
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onOpen()
{
    if (isDirty()) {
        const auto r = QMessageBox::question(
            this,
            tr("Ungespeicherte Änderungen"),
            tr("Es gibt ungespeicherte Änderungen. Willst du vorher speichern?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Yes);

        if (r == QMessageBox::Cancel)
            return;
        if (r == QMessageBox::Yes)
            onSave();
    }

    QSettings s(kOrgName, kAppName);
    const QString lastDir = s.value(QString("%1/lastDir").arg(kGroup), QDir::homePath()).toString();

    const QString fn = QFileDialog::getOpenFileName(
        this,
        tr("Open file"),
        lastDir,
        tr("Scripts (*.txt *.ada *.neo *.lua *.js);;Alle Dateien (*.*)"));

    if (fn.isEmpty())
        return;

    s.setValue(QString("%1/lastDir").arg(kGroup), QFileInfo(fn).absolutePath());

    mCurrentFileName = fn;
    loadCurrentFile();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onSave()
{
    if (mCurrentFileName.isEmpty()) {
        onSaveAs();
        return;
    }

    if (!ui->txtScript)
        return;

    // QSaveFile schreibt atomar (temp + rename) -> sicherer als QFile
    QSaveFile f(mCurrentFileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this,
                             tr("Speichern fehlgeschlagen"),
                             tr("Datei konnte nicht zum Schreiben geöffnet werden:\n%1")
                                 .arg(mCurrentFileName));
        return;
    }

    QTextStream out(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif

    out << ui->txtScript->toPlainText();

    if (!f.commit()) {
        QMessageBox::warning(this,
                             tr("Speichern fehlgeschlagen"),
                             tr("Datei konnte nicht final gespeichert werden:\n%1")
                                 .arg(mCurrentFileName));
        return;
    }

    // Optional: Titel/Status
    // setWindowTitle(QFileInfo(mCurrentFileName).fileName());
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onSaveAs()
{
    if (!ui->txtScript)
        return;

    QSettings s(kOrgName, kAppName);
    const QString lastDir = s.value(QString("%1/lastDir").arg(kGroup), QDir::homePath()).toString();

    const QString fn = QFileDialog::getSaveFileName(
        this,
        tr("Save as..."),
        lastDir,
        tr("Scripts (*.txt *.ada);;all (*.*)"));

    if (fn.isEmpty())
        return;

    s.setValue(QString("%1/lastDir").arg(kGroup), QFileInfo(fn).absolutePath());

    mCurrentFileName = fn;
    onSave();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onPlay()
{
    initRuntime();
    ui->txtOutput->clear();
    auto ret = mAda.runScript(ui->txtScript->toPlainText().toStdString());

    if (mAda.hasError())
        ui->txtOutput->appendPlainText(QString::fromStdString(mAda.lastError()));
    else if (ret.type() != Nda::Undefined)
        ui->txtOutput->appendPlainText("Script returns: '" + QString::fromStdString(ret.toString()) + "'");
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onStop()
{

}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::closeEvent(QCloseEvent *event)
{
    if (isDirty()) {
        const auto r = QMessageBox::question(
            this,
            tr("Ungespeicherte Änderungen"),
            tr("Es gibt ungespeicherte Änderungen. Willst du vorher speichern?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Yes);

        if (r == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if (r == QMessageBox::Yes)
            onSave();
    }
    event->accept();
}

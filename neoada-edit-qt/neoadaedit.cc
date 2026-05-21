#include "neoadaedit.h"
#include "ui_neoadaedit.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSaveFile>
#include <QSettings>
#include <QShortcut>
#include <QSplitter>
#include <QStyle>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

#include "neoadahighlighter.h"
#include <state.h>
#include <runtime.h>

static const char* kOrgName  = "Solarix95";
static const char* kAppName  = "NeoAda";
static const char* kGroup    = "NeoAdaEdit";

static void appendOutputLine(QPlainTextEdit *output, const QString &line, const QColor &color = QColor())
{
    QTextCursor cursor(output->document());
    cursor.movePosition(QTextCursor::End);
    if (!output->document()->isEmpty())
        cursor.insertBlock();

    QTextCharFormat format;
    if (color.isValid())
        format.setForeground(color);
    cursor.insertText(line, format);

    output->setTextCursor(cursor);
    output->ensureCursorVisible();
}

static QString displayExceptionName(QString name)
{
    const QString lower = name.toLower();
    if (lower == QStringLiteral("constrainterror"))
        return QStringLiteral("ConstraintError");
    if (lower == QStringLiteral("programerror"))
        return QStringLiteral("ProgramError");
    return name;
}

static QString defaultScript()
{
    return QString::fromUtf8(R"(-- Willkommen bei NeoAda
-- Starte mit F5 oder dem Run-Knopf.

declare name : String := "NeoAda";
declare n : Natural := 3;

while n > 0 loop
    print(name & " sagt Hallo " & n);
    n := n - 1;
end loop;

return "fertig";
)");
}

//-------------------------------------------------------------------------------------------------
NeoAdaEdit::NeoAdaEdit(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NeoAdaEdit)
    , mHighlighter(nullptr)
    , mAda(*(new NdaRuntime()))
    , mExampleBox(nullptr)
    , mGuide(nullptr)
    , mMainSplitter(nullptr)
    , mStatus(nullptr)
    , mNewButton(nullptr)
    , mLoadExampleButton(nullptr)
    , mBufferedOutput(nullptr)
    , mRunning(false)
{
    ui->setupUi(this);

    connect(ui->btnOpen, &QPushButton::clicked, this, &NeoAdaEdit::onOpen);
    connect(ui->btnSave, &QPushButton::clicked, this, &NeoAdaEdit::onSave);
    connect(ui->btnSaveAs, &QPushButton::clicked, this, &NeoAdaEdit::onSaveAs);
    connect(ui->btnPlay, &QPushButton::clicked, this, &NeoAdaEdit::onPlay);
    connect(ui->btnStop, &QPushButton::clicked, this, &NeoAdaEdit::onStop);

    initExamples();
    initEditor();
    initLearningTools();

    connect(ui->txtScript, &QTextEdit::textChanged, this, &NeoAdaEdit::updateTitle);

    new QShortcut(QKeySequence::New, this, SLOT(onNew()));
    new QShortcut(QKeySequence::Open, this, SLOT(onOpen()));
    new QShortcut(QKeySequence::Save, this, SLOT(onSave()));
    new QShortcut(QKeySequence(Qt::Key_F5), this, SLOT(onPlay()));

    if (ui->txtScript->toPlainText().isEmpty())
        setScriptText(defaultScript());

    updateTitle();
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
    s.setValue("geometry", saveGeometry());
    s.setValue("splitter", ui->splitter->saveState());
    if (mMainSplitter)
        s.setValue("mainSplitter", mMainSplitter->saveState());

    if (ui->txtScript) {
        s.setValue("cursorPosition", ui->txtScript->textCursor().position());
        s.setValue("plainText", ui->txtScript->toPlainText());
    }

    s.endGroup();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::restoreState()
{
    QSettings s(kOrgName, kAppName);
    s.beginGroup(kGroup);

    restoreGeometry(s.value("geometry").toByteArray());
    ui->splitter->restoreState(s.value("splitter").toByteArray());
    if (mMainSplitter)
        mMainSplitter->restoreState(s.value("mainSplitter").toByteArray());

    mCurrentFileName = s.value("currentFileName").toString();

    const QString lastText = s.value("plainText").toString();
    if (!lastText.isEmpty() && ui->txtScript) {
        ui->txtScript->setPlainText(lastText);
        auto c = ui->txtScript->textCursor();
        c.setPosition(s.value("cursorPosition", 0).toInt());
        ui->txtScript->setTextCursor(c);
    }

    s.endGroup();

    if (!mCurrentFileName.isEmpty())
        loadCurrentFile();

    updateTitle();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::setRunning(bool running)
{
    mRunning = running;

    ui->btnOpen->setEnabled(!running);
    ui->btnSave->setEnabled(!running);
    ui->btnSaveAs->setEnabled(!running);
    ui->btnPlay->setEnabled(!running);
    ui->btnStop->setEnabled(!running);
    ui->txtScript->setReadOnly(running);

    if (mNewButton)
        mNewButton->setEnabled(!running);
    if (mLoadExampleButton)
        mLoadExampleButton->setEnabled(!running);
    if (mExampleBox)
        mExampleBox->setEnabled(!running);
    if (mBufferedOutput)
        mBufferedOutput->setEnabled(!running);

    if (mStatus)
        mStatus->setText(running ? tr("Running script...") : tr("Ready"));
}

//-------------------------------------------------------------------------------------------------
bool NeoAdaEdit::confirmDiscardChanges()
{
    if (mRunning)
        return false;

    if (!isDirty())
        return true;

    const auto r = QMessageBox::question(
        this,
        tr("Ungespeicherte Änderungen"),
        tr("Es gibt ungespeicherte Änderungen. Willst du vorher speichern?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Yes);

    if (r == QMessageBox::Cancel)
        return false;
    if (r == QMessageBox::Yes)
        onSave();

    return true;
}

//-------------------------------------------------------------------------------------------------
bool NeoAdaEdit::isDirty() const
{
    if (!ui->txtScript)
        return false;

    if (mCurrentFileName.isEmpty())
        return !ui->txtScript->toPlainText().isEmpty();

    QFile f(mCurrentFileName);
    if (!f.exists())
        return true;

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return true;

    QTextStream in(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    in.setCodec("UTF-8");
#endif

    return in.readAll() != ui->txtScript->toPlainText();
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
                             tr("Datei konnte nicht geöffnet werden:\n%1").arg(mCurrentFileName));
        return;
    }

    QTextStream in(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    in.setCodec("UTF-8");
#endif

    ui->txtScript->setPlainText(in.readAll());
    updateTitle();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::initRuntime()
{
    mAda.reset();
    mAda.state()->bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        const QString line = QString::fromStdString(args.at("message").toString());
        const bool buffered = !mBufferedOutput || mBufferedOutput->isChecked();
        if (buffered) {
            mRunOutput.append(line);
        } else {
            ui->txtOutput->appendPlainText(line);
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        return true;
    });
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::initEditor()
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(11);
    ui->txtScript->setFont(font);
    ui->txtOutput->setFont(font);

    ui->txtScript->setLineWrapMode(QTextEdit::NoWrap);
    ui->txtOutput->setMaximumBlockCount(5000);
    ui->txtOutput->setPlaceholderText(tr("Ausgabe von print(), Rückgabewerte und Fehler erscheinen hier."));

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    const int spaceWidth = QFontMetrics(font).horizontalAdvance(' ');
#else
    const int spaceWidth = QFontMetrics(font).width(' ');
#endif
    ui->txtScript->setTabStopDistance(spaceWidth * 4);

    ui->label->setText(tr("NeoAda Script"));
    ui->txtScript->setPlaceholderText(tr("Schreibe NeoAda-Code oder lade links oben ein Beispiel."));

    ui->btnOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->btnSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->btnSaveAs->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->btnStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->btnPlay->setText(tr("Run"));
    ui->btnStop->setText(tr("Clear"));

    mHighlighter = new NeoAdaHighlighter(ui->txtScript->document());
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::initExamples()
{
    mExamples.clear();
    mExamples.push_back({tr("Hallo Welt"), tr("Anfänger"), QString::fromUtf8(R"(print("Hallo NeoAda!");
return "ok";
)")});

    mExamples.push_back({tr("Variablen und Schleifen"), tr("Anfänger"), QString::fromUtf8(R"(declare n : Natural := 1;

declare sum : Natural := 0;
while n <= 10 loop
    sum := sum + n;
    n := n + 1;
end loop;

print("Summe 1..10 = " & sum);
return sum;
)")});

    mExamples.push_back({tr("Eigene Funktion"), tr("Fortgeschritten"), QString::fromUtf8(R"(function square(x : Natural) return Natural is
begin
    return x * x;
end;

for i in 1..5 loop
    print(i & "^2 = " & square(i));
end loop;
)")});

    mExamples.push_back({tr("Listen"), tr("Fortgeschritten"), QString::fromUtf8(R"(with Ada.List;

declare xs : List := [3,1,4];
xs.append(1);
xs.insert(1, 9);

print("length=" & xs.length());
return xs;
)")});

    mExamples.push_back({tr("Dictionaries"), tr("Fortgeschritten"), QString::fromUtf8(R"(declare person : Dict := {"name":"Ada", "age":12};

person{"language"} := "NeoAda";
person{"stats"} := {"projects":3, "level":"advanced"};

print(person{"name"} & " lernt " & person{"language"});
print("Projekte: " & person{"stats"}{"projects"});

return person{"age"};
)")});

    mExamples.push_back({tr("Exceptions"), tr("Fortgeschritten"), QString::fromUtf8(R"(function riskyDivide(a : Natural; b : Natural) return Natural is
begin
    return a / b;
exception
    when ConstraintError =>
        print("Division nicht moeglich");
        raise;
end;

function safeDivide(a : Natural; b : Natural) return Natural is
begin
    return riskyDivide(a, b);
exception
    when ConstraintError =>
        print("Fallback auf 0");
        return 0;
end;

return safeDivide(42, 0);
)")});

    mExamples.push_back({tr("Textdatei schreiben"), tr("Praxis"), QString::fromUtf8(R"(with Ada.Io.File;

declare f : TextFile := TextFile:create("/tmp/neoada_hello.txt");
f.writeLine("Hallo aus NeoAda");
f.writeLine("TextFile arbeitet mit String.");
f.close();

print("/tmp/neoada_hello.txt erstellt");
)")});

    mExamples.push_back({tr("DateTime"), tr("Praxis"), QString::fromUtf8(R"(with Ada.DateTime;

declare start : DateTime := DateTime:fromString("2026-05-21 09:30:00", "yyyy-MM-dd HH:mm:ss");
declare deadline : DateTime := start.addDays(3).addSecs(2 * 60 * 60);

print("Start:    " & start.toString("dd.MM.yyyy HH:mm"));
print("Deadline: " & deadline.toString("dd.MM.yyyy HH:mm"));

declare today : Date := Date:now();
print("Heute:   " & today.toString("dd.MM.yyyy"));

return deadline.toString("yyyy-MM-dd HH:mm:ss");
)")});

    mExamples.push_back({tr("Bytes und PNG"), tr("Advanced"), QString::fromUtf8(R"(with Ada.Bytes;
with Ada.Io.File;

declare data : Bytes;
data.append(137_b);
data.append(80_b);
data.append(78_b);
data.append(71_b);

print("PNG-Signatur Länge: " & data.length());
print("Erstes Byte: " & data[0]);
)")});

    mExamples.push_back({tr("Eigener Typ"), tr("Advanced"), QString::fromUtf8(R"(type Counter is Natural;

function Counter:next return Natural is
begin
    return this + 1;
end;

declare c : Counter := 41;
return c.next();
)")});
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::initLearningTools()
{
    mNewButton = new QPushButton(tr("New"), this);
    mNewButton->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    connect(mNewButton, &QPushButton::clicked, this, &NeoAdaEdit::onNew);
    ui->horizontalLayout->insertWidget(0, mNewButton);

    auto *bar = new QFrame(this);
    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 0, 0, 0);

    mBufferedOutput = new QCheckBox(tr("Buffered output"), bar);
    mBufferedOutput->setChecked(true);
    mBufferedOutput->setToolTip(tr("Buffered: output is shown after the script ends. Unchecked: print() output is shown while running; user input stays blocked."));

    mStatus = new QLabel(tr("F5 runs the script"), bar);
    mStatus->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(mBufferedOutput);
    layout->addWidget(mStatus, 1);

    ui->verticalLayout->insertWidget(1, bar);

    auto *guidePanel = new QWidget(this);
    auto *guideLayout = new QVBoxLayout(guidePanel);
    guideLayout->setContentsMargins(0, 0, 0, 0);

    auto *exampleBar = new QFrame(guidePanel);
    auto *exampleLayout = new QHBoxLayout(exampleBar);
    exampleLayout->setContentsMargins(0, 0, 0, 0);

    mExampleBox = new QComboBox(exampleBar);
    for (const auto &ex : mExamples)
        mExampleBox->addItem(QString("%1 - %2").arg(ex.level, ex.title));

    mLoadExampleButton = new QPushButton(tr("Load Example"), exampleBar);
    mLoadExampleButton->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    connect(mLoadExampleButton, &QPushButton::clicked, this, &NeoAdaEdit::onLoadExample);

    exampleLayout->addWidget(mExampleBox, 1);
    exampleLayout->addWidget(mLoadExampleButton);

    mGuide = new QTextBrowser(guidePanel);
    mGuide->setOpenExternalLinks(false);
    mGuide->setHtml(tr(R"(
<h3>NeoAda Quick Guide</h3>
<p><b>Start:</b> Write code, press <b>F5</b>, read the output below.</p>
<ul>
<li><code>declare n : Natural := 42;</code></li>
<li><code>print("n=" & n);</code></li>
<li><code>if n &gt; 0 then ... end if;</code></li>
<li><code>while n &gt; 0 loop ... end loop;</code></li>
<li><code>for i in 1..10 loop ... end loop;</code></li>
</ul>
<p><b>Addons:</b></p>
<ul>
<li><code>with Ada.List;</code> for <code>List.length()</code>, <code>append()</code>, ...</li>
<li><code>with Ada.String;</code> for text helpers.</li>
<li><code>with Ada.Bytes;</code> for binary byte arrays.</li>
<li><code>with Ada.Io.File;</code> for <code>File</code> and <code>TextFile</code>.</li>
</ul>
<p><b>Exceptions:</b></p>
<ul>
<li><code>exception when ConstraintError =&gt; print("Index oder Zahl ungueltig");</code></li>
<li><code>when others =&gt; print("Fehler"); raise;</code> re-raises the active exception.</li>
</ul>
<p><b>Files:</b> <code>TextFile</code> reads/writes <code>String</code>; <code>File</code> reads/writes <code>Bytes</code>.</p>
<p><b>Tip:</b> Use examples as small experiments. Change one line, run again, compare output.</p>
)"));
    mGuide->setMinimumWidth(240);

    guideLayout->addWidget(exampleBar);
    guideLayout->addWidget(mGuide, 1);

    auto *content = ui->splitter;
    ui->verticalLayout_2->removeWidget(content);

    mMainSplitter = new QSplitter(Qt::Horizontal, this);
    mMainSplitter->addWidget(content);
    mMainSplitter->addWidget(guidePanel);
    mMainSplitter->setStretchFactor(0, 1);
    mMainSplitter->setStretchFactor(1, 0);
    mMainSplitter->setSizes({760, 280});

    ui->verticalLayout_2->addWidget(mMainSplitter);
    ui->splitter->setSizes({420, 140});
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::setScriptText(const QString &text, const QString &fileName)
{
    mCurrentFileName = fileName;
    ui->txtScript->setPlainText(text);
    ui->txtScript->moveCursor(QTextCursor::Start);
    updateTitle();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::updateTitle()
{
    const QString name = mCurrentFileName.isEmpty() ? tr("Untitled") : QFileInfo(mCurrentFileName).fileName();
    setWindowTitle(QString("%1%2 - NeoAda Edit").arg(name, isDirty() ? " *" : ""));
    if (mStatus) {
        const int lines = ui->txtScript->document()->blockCount();
        mStatus->setText(tr("%1 lines | F5 Run").arg(lines));
    }
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onNew()
{
    if (mRunning)
        return;
    if (!confirmDiscardChanges())
        return;
    setScriptText(defaultScript());
    ui->txtOutput->clear();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onOpen()
{
    if (mRunning)
        return;
    if (!confirmDiscardChanges())
        return;

    QSettings s(kOrgName, kAppName);
    const QString lastDir = s.value(QString("%1/lastDir").arg(kGroup), QDir::homePath()).toString();

    const QString fn = QFileDialog::getOpenFileName(
        this,
        tr("Open file"),
        lastDir,
        tr("NeoAda Scripts (*.ada *.neo *.txt);;All files (*.*)"));

    if (fn.isEmpty())
        return;

    s.setValue(QString("%1/lastDir").arg(kGroup), QFileInfo(fn).absolutePath());

    mCurrentFileName = fn;
    loadCurrentFile();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onSave()
{
    if (mRunning)
        return;

    if (mCurrentFileName.isEmpty()) {
        onSaveAs();
        return;
    }

    if (!ui->txtScript)
        return;

    QSaveFile f(mCurrentFileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this,
                             tr("Speichern fehlgeschlagen"),
                             tr("Datei konnte nicht zum Schreiben geöffnet werden:\n%1").arg(mCurrentFileName));
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
                             tr("Datei konnte nicht final gespeichert werden:\n%1").arg(mCurrentFileName));
        return;
    }

    updateTitle();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onSaveAs()
{
    if (mRunning)
        return;

    if (!ui->txtScript)
        return;

    QSettings s(kOrgName, kAppName);
    const QString lastDir = s.value(QString("%1/lastDir").arg(kGroup), QDir::homePath()).toString();

    const QString fn = QFileDialog::getSaveFileName(
        this,
        tr("Save as..."),
        lastDir,
        tr("NeoAda Scripts (*.ada *.neo);;All files (*.*)"));

    if (fn.isEmpty())
        return;

    s.setValue(QString("%1/lastDir").arg(kGroup), QFileInfo(fn).absolutePath());

    mCurrentFileName = fn;
    onSave();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onPlay()
{
    if (mRunning)
        return;

    mRunOutput.clear();
    initRuntime();
    ui->txtOutput->clear();
    setRunning(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (mBufferedOutput && !mBufferedOutput->isChecked())
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    QElapsedTimer timer;
    timer.start();
    auto ret = mAda.runScript(ui->txtScript->toPlainText().toStdString());
    const qint64 elapsed = timer.elapsed();

    QApplication::restoreOverrideCursor();
    setRunning(false);

    if (!mBufferedOutput || mBufferedOutput->isChecked()) {
        for (const auto &line : mRunOutput)
            ui->txtOutput->appendPlainText(line);
    }

    const QString unhandledException = QString::fromStdString(mAda.state()->unhandledException());
    if (mAda.hasError()) {
        appendOutputLine(ui->txtOutput, QString("Error: %1").arg(QString::fromStdString(mAda.lastError())), QColor("#b00020"));
        if (mStatus)
            mStatus->setText(tr("Error after %1 ms").arg(elapsed));
    } else if (!unhandledException.isEmpty()) {
        appendOutputLine(ui->txtOutput, tr("Unhandled exception: %1").arg(displayExceptionName(unhandledException)), QColor("#b00020"));
        appendOutputLine(ui->txtOutput, tr("Finished in %1 ms").arg(elapsed));
        if (mStatus)
            mStatus->setText(tr("Unhandled exception after %1 ms").arg(elapsed));
    } else {
        if (ret.type() != Nda::Undefined)
            ui->txtOutput->appendPlainText("\nreturn: " + QString::fromStdString(ret.toString()));
        ui->txtOutput->appendPlainText(tr("\nFinished in %1 ms").arg(elapsed));
        if (mStatus)
            mStatus->setText(tr("Finished in %1 ms").arg(elapsed));
    }
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onStop()
{
    if (mRunning)
        return;

    ui->txtOutput->clear();
    if (mStatus)
        mStatus->setText(tr("Output cleared"));
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onLoadExample()
{
    if (mRunning)
        return;

    if (!mExampleBox)
        return;
    if (!confirmDiscardChanges())
        return;

    const int index = mExampleBox->currentIndex();
    if (index < 0 || index >= mExamples.size())
        return;

    setScriptText(mExamples[index].code);
    ui->txtOutput->clear();
    ui->txtOutput->appendPlainText(tr("Loaded example: %1").arg(mExamples[index].title));
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::closeEvent(QCloseEvent *event)
{
    if (mRunning) {
        event->ignore();
        return;
    }

    if (!confirmDiscardChanges()) {
        event->ignore();
        return;
    }

    saveState();
    event->accept();
}

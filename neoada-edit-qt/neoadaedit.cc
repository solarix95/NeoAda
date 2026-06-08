#include "neoadaedit.h"
#include "ui_neoadaedit.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
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
#include <QRadioButton>
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
#include "scenarios/abstractscenario.h"
#include "scenarios/asteroiddefensescenario.h"
#include "scenarios/marsroverscenario.h"
#include "scenarios/rocketscenario.h"
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
    , mGuide(nullptr)
    , mMainSplitter(nullptr)
    , mActiveScenario(nullptr)
    , mNewButton(nullptr)
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
    initScenarios();
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
    qDeleteAll(mScenarios);
    mScenarios.clear();
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
    if (mBufferedOutput)
        mBufferedOutput->setEnabled(!running);

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
    resetScenarios();
    mAda.state()->onWith([this](std::string &addonName) {
        loadEditorAddon(addonName);
    });
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

    ui->label->hide();
    ui->txtScript->setPlaceholderText(tr("Schreibe NeoAda-Code oder starte über New ein Beispiel oder Szenario."));

    ui->btnOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->btnSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->btnSaveAs->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->btnStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->btnPlay->setText(tr("Run"));
    ui->btnPlay->setToolTip(tr("F5 - Run"));
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

    mExamples.push_back({tr("Datum/Uhrzeit"), tr("Anfänger"), QString::fromUtf8(R"(with Ada.DateTime;

declare today : Date := Date:now();
declare now : Time := Time:now();
declare meeting : DateTime := DateTime:fromString("2026-05-22 14:30:00", "yyyy-MM-dd HH:mm:ss");

print("Heute: " & today.toString("dd.MM.yyyy"));
print("Jetzt: " & now.toString("HH:mm:ss"));
print("Termin: " & meeting.toString("dd.MM.yyyy HH:mm"));

return today.toString("yyyy-MM-dd");
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


    mExamples.push_back({tr("Math"), tr("Fortgeschritten"), QString::fromUtf8(R"(with Ada.Math;

declare angle : Number := Math:radians(45);
declare eps : Number := 0.000001;
declare side : Number := Math:sqrt(2) / 2;

print("pi = " & Math:pi());
print("sin(45 Grad) = " & Math:sin(angle));
print("cos(45 Grad) = " & Math:cos(angle));

if Math:abs(Math:sin(angle) - side) < eps and Math:abs(Math:cos(angle) - side) < eps then
    return "ok";
end if;

return "unerwartet";
)")});

    mExamples.push_back({tr("Regexp"), tr("Fortgeschritten"), QString::fromUtf8(R"NEOADA(with Ada.Regexp;

declare text : String := "Name: Ada, Alter: 12";
declare caps : List := Regexp:captures(text, "Name: ([A-Za-z]+), Alter: ([0-9]+)");
declare normalized : String := Regexp:replace("Ada   lernt    NeoAda", "[ ]+", " ");

if Regexp:contains(text, "Alter") and #caps = 3 then
    print("Name: " & caps[1]);
    print("Alter: " & caps[2]);
    print(normalized);
    return caps[1];
end if;

return "kein Treffer";
)NEOADA")});

    mExamples.push_back({tr("JSON"), tr("Fortgeschritten"), QString::fromUtf8(R"NEOADA(with Ada.Json;
with Ada.Io.File;

declare person : Dict := {"name":"Ada", "age":12, "skills":["NeoAda", "Qt"]};
declare text : String := Json:toString(person);
print(text);

declare parsed : Dict := Json:fromString(text);
print(parsed{"name"} & " lernt " & parsed{"skills"}[0]);

declare fileOut : TextFile := TextFile:create("/tmp/neoada_person.json");
Json:write(fileOut, parsed);
fileOut.close();

declare input : TextFile := TextFile:openRead("/tmp/neoada_person.json");
declare fromFile : Dict := Json:read(input);
input.close();

return fromFile{"age"};
)NEOADA")});

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
void NeoAdaEdit::initScenarios()
{
    qDeleteAll(mScenarios);
    mScenarios.clear();
    mScenarios.push_back(new MarsRoverScenario());
    mScenarios.push_back(new RocketScenario());
    mScenarios.push_back(new AsteroidDefenseScenario());
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::resetScenarios()
{
    mActiveScenario = nullptr;
    for (auto *scenario : mScenarios)
        scenario->reset();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::activateScenario(const QString &packageName)
{
    const QString normalized = packageName.toLower();
    for (auto *scenario : mScenarios) {
        if (scenario->packageName().toLower() != normalized)
            continue;

        mActiveScenario = scenario;
        QWidget *scenarioWindow = scenario->widget();
        scenarioWindow->setWindowTitle(scenario->title());
        scenarioWindow->setAttribute(Qt::WA_QuitOnClose, false);
        if (!scenarioWindow->isVisible())
            scenarioWindow->show();
        scenarioWindow->raise();
        scenarioWindow->activateWindow();
        scenario->bind(mAda);
        return;
    }
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::loadEditorAddon(const std::string &addonName)
{
    if (addonName == "ada.list")
        mAda.loadAddonAdaList();
    else if (addonName == "ada.dict")
        mAda.loadAddonAdaDict();
    else if (addonName == "ada.bytes")
        mAda.loadAddonAdaBytes();
    else if (addonName == "ada.string")
        mAda.loadAddonAdaString();
    else if (addonName == "ada.math")
        mAda.loadAddonAdaMath();
    else if (addonName == "ada.text.encoding")
        mAda.loadAddonAdaTextEncoding();
    else if (addonName == "ada.io.file" || addonName == "ada.io")
        mAda.loadAddonAdaIoFile();
    else if (addonName == "ada.datetime" || addonName == "ada.date.time" || addonName == "ada.date")
        mAda.loadAddonAdaDateTime();
    else if (addonName == "ada.regexp" || addonName == "ada.regex")
        mAda.loadAddonAdaRegexp();
    else if (addonName == "ada.json")
        mAda.loadAddonAdaJson();
    else if (QString::fromStdString(addonName).startsWith(QStringLiteral("app.neoadaedit.")))
        activateScenario(QString::fromStdString(addonName));
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::initLearningTools()
{
    mNewButton = new QPushButton(tr("New"), this);
    mNewButton->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    connect(mNewButton, &QPushButton::clicked, this, &NeoAdaEdit::onNew);

    mBufferedOutput = new QCheckBox(tr("Buffered output"), this);
    mBufferedOutput->setChecked(true);
    mBufferedOutput->setToolTip(tr("Buffered: output is shown after the script ends. Unchecked: print() output is shown while running; user input stays blocked."));

    auto *controls = ui->horizontalLayout;
    QLayoutItem *item = nullptr;
    while ((item = controls->takeAt(0)) != nullptr)
        delete item;

    controls->setContentsMargins(0, 0, 0, 0);

    auto *leftControls = new QWidget(this);
    auto *leftLayout = new QHBoxLayout(leftControls);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->addWidget(mNewButton);
    leftLayout->addWidget(ui->btnOpen);
    leftLayout->addWidget(ui->btnSave);
    leftLayout->addWidget(ui->btnSaveAs);
    leftLayout->addStretch(1);

    auto *runControls = new QWidget(this);
    auto *runLayout = new QHBoxLayout(runControls);
    runLayout->setContentsMargins(0, 0, 0, 0);
    runLayout->addStretch(1);
    runLayout->addWidget(ui->btnPlay);
    runLayout->addWidget(ui->btnStop);
    runLayout->addStretch(1);

    auto *rightControls = new QWidget(this);
    auto *rightLayout = new QHBoxLayout(rightControls);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addStretch(1);
    rightLayout->addWidget(mBufferedOutput);

    controls->addWidget(leftControls, 1);
    controls->addWidget(runControls, 1);
    controls->addWidget(rightControls, 1);

    auto *guidePanel = new QWidget(this);
    auto *guideLayout = new QVBoxLayout(guidePanel);
    guideLayout->setContentsMargins(0, 0, 0, 0);

    mGuide = new QTextBrowser(guidePanel);
    mGuide->setOpenExternalLinks(false);
    mGuide->setHtml(tr(R"(
<h3>NeoAda Quick Guide</h3>
<p><b>Start:</b> Write code, press <b>F5</b>, read the output below.</p>
<ul>
<li><code>declare n : Natural := 42;</code></li>
<li><code>print("n=" &amp; n);</code></li>
<li><code>if n &gt; 0 then ... end if;</code></li>
<li><code>while n &gt; 0 loop ... end loop;</code></li>
<li><code>for i in 1..10 loop ... end loop;</code></li>
</ul>

<p><b>Case / when:</b></p>
<pre>case month is
  when 1 =&gt; print("January");
  when 2 =&gt; print("February");
  when others =&gt; print("Other");
end case;</pre>

<p><b>Numeric literals:</b></p>
<ul>
<li><code>42</code>, <code>42_n</code>: <code>Natural</code></li>
<li><code>42_u</code>: <code>Supernatural</code> (unsigned)</li>
<li><code>42_d</code>, <code>3.14</code>, <code>1.2E3</code>: <code>Number</code></li>
<li><code>255_b</code>: <code>Byte</code></li>
<li><code>1_000_000</code>: digit separators</li>
<li><code>16#FF#</code>, <code>2#1010#</code>: based literals</li>
</ul>

<p><b>Addons:</b> Load helpers with <code>with Ada.Name;</code>.</p>

<p><b>Ada.String</b></p>
<p><code>length()</code>, <code>append(value)</code>, <code>insert(pos,text)</code><br>
<code>contains(text)</code>, <code>indexOf(text)</code><br>
<code>toUpper()</code>, <code>toLower()</code>, <code>upper()</code>, <code>lower()</code><br>
<code>trim()</code>, <code>trimmed()</code>, <code>chop(n)</code>, <code>chopped(n)</code><br>
<code>slice(pos,n)</code>, <code>sliced(pos,n)</code><br>
<code>toNumber()</code>, <code>toNatural()</code>, <code>toBool()</code><br>
<code>isNumber()</code>, <code>isNatural()</code>, <code>isBool()</code><br>
<code>String:format(value,"f2")</code><br>
<code>String:fromBytes(data,encoding)</code>, <code>toBytes(encoding)</code></p>

<p><b>Ada.List</b></p>
<p>Access elements with <code>list[index]</code>.<br>
<code>length()</code>, <code>clear()</code>, <code>append(value)</code><br>
<code>insert(pos,value)</code>, <code>concat(value)</code><br>
<code>removeAt(pos)</code>, <code>removeFirst()</code>, <code>removeLast()</code><br>
<code>contains(value)</code>, <code>indexOf(value)</code><br>
<code>flip()</code>, <code>flipped()</code></p>

<p><b>Ada.Dict</b></p>
<p>Access values with <code>dict{key}</code>.<br>
<code>length()</code>, <code>clear()</code>, <code>contains(key)</code><br>
<code>remove(key)</code>, <code>keys()</code>, <code>values()</code><br>
<code>value(key,defaultValue)</code></p>

<p><b>Other addons:</b></p>
<ul>
<li><code>with Ada.Bytes;</code> for binary byte arrays.</li>
<li><code>with Ada.Io.File;</code> for <code>File</code> and <code>TextFile</code>.</li>
<li><code>with Ada.Math;</code>, <code>Ada.DateTime</code>, <code>Ada.Json</code>, <code>Ada.Regexp</code>.</li>
</ul>

<p><b>Exceptions:</b></p>
<pre>begin
  -- risky code
exception
  when ConstraintError =&gt; print("Invalid value");
  when others =&gt; print("Error"); raise;
end;</pre>
<p><b>Tip:</b> Use <b>New</b> to start with an empty script, an example, or a prepared scenario.</p>
)"));
    mGuide->setMinimumWidth(240);

    guideLayout->addWidget(mGuide, 1);

    auto *content = ui->splitter;
    ui->verticalLayout_2->removeWidget(content);

    for (auto *scenario : mScenarios) {
        scenario->widget()->setParent(nullptr);
        scenario->widget()->setAttribute(Qt::WA_QuitOnClose, false);
        scenario->widget()->hide();
    }

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
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onNew()
{
    if (mRunning)
        return;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("New"));

    auto *layout = new QVBoxLayout(&dialog);

    QSettings settings(kOrgName, kAppName);
    const QString lastNewMode = settings.value(QString("%1/newMode").arg(kGroup), QStringLiteral("empty")).toString();
    const int lastExampleIndex = settings.value(QString("%1/newExampleIndex").arg(kGroup), 0).toInt();
    const int lastScenarioIndex = settings.value(QString("%1/newScenarioIndex").arg(kGroup), 0).toInt();

    auto *emptyRadio = new QRadioButton(tr("empty script"), &dialog);
    auto *emptyLabel = new QLabel(tr("Starte ein neues Script."), &dialog);
    emptyLabel->setIndent(24);

    auto *exampleRadio = new QRadioButton(tr("Example"), &dialog);
    auto *exampleBox = new QComboBox(&dialog);
    for (const auto &ex : mExamples)
        exampleBox->addItem(QString("%1 - %2").arg(ex.level, ex.title));
    if (exampleBox->count() > 0)
        exampleBox->setCurrentIndex(qBound(0, lastExampleIndex, exampleBox->count() - 1));

    auto *scenarioRadio = new QRadioButton(tr("Scenario"), &dialog);
    auto *scenarioBox = new QComboBox(&dialog);
    for (const auto *scenario : mScenarios) {
        scenarioBox->addItem(scenario->title());
        scenarioBox->setItemData(scenarioBox->count() - 1, scenario->description(), Qt::ToolTipRole);
    }
    if (scenarioBox->count() > 0)
        scenarioBox->setCurrentIndex(qBound(0, lastScenarioIndex, scenarioBox->count() - 1));

    if (lastNewMode == QStringLiteral("example"))
        exampleRadio->setChecked(true);
    else if (lastNewMode == QStringLiteral("scenario"))
        scenarioRadio->setChecked(true);
    else
        emptyRadio->setChecked(true);

    exampleBox->setEnabled(exampleRadio->isChecked());
    scenarioBox->setEnabled(scenarioRadio->isChecked());

    connect(exampleRadio, &QRadioButton::toggled, exampleBox, &QComboBox::setEnabled);
    connect(scenarioRadio, &QRadioButton::toggled, scenarioBox, &QComboBox::setEnabled);

    layout->addWidget(emptyRadio);
    layout->addWidget(emptyLabel);
    layout->addWidget(exampleRadio);
    layout->addWidget(exampleBox);
    layout->addWidget(scenarioRadio);
    layout->addWidget(scenarioBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString code = defaultScript();
    QString loadedTitle;
    QString selectedMode = QStringLiteral("empty");
    if (exampleRadio->isChecked()) {
        const int index = exampleBox->currentIndex();
        if (index < 0 || index >= mExamples.size())
            return;
        code = mExamples[index].code;
        loadedTitle = tr("Example: %1").arg(mExamples[index].title);
        selectedMode = QStringLiteral("example");
    } else if (scenarioRadio->isChecked()) {
        const int index = scenarioBox->currentIndex();
        if (index < 0 || index >= mScenarios.size())
            return;
        code = mScenarios[index]->initialSource();
        loadedTitle = tr("Scenario: %1").arg(mScenarios[index]->title());
        selectedMode = QStringLiteral("scenario");
    }

    settings.setValue(QString("%1/newMode").arg(kGroup), selectedMode);
    settings.setValue(QString("%1/newExampleIndex").arg(kGroup), exampleBox->currentIndex());
    settings.setValue(QString("%1/newScenarioIndex").arg(kGroup), scenarioBox->currentIndex());

    if (!confirmDiscardChanges())
        return;

    setScriptText(code);
    ui->txtOutput->clear();
    if (!loadedTitle.isEmpty())
        ui->txtOutput->appendPlainText(tr("Loaded %1").arg(loadedTitle));
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

    saveState();

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

    const QString unhandledException = QString::fromStdString(mAda.state()->unhandledException());
    if (!mAda.hasError() && unhandledException.isEmpty()) {
        for (auto *scenario : mScenarios) {
            if (scenario->widget()->isVisible())
                scenario->afterRun(mAda);
        }
    }

    if (!mBufferedOutput || mBufferedOutput->isChecked()) {
        for (const auto &line : mRunOutput)
            ui->txtOutput->appendPlainText(line);
    }

    /*
    for (const auto &sym : mAda.globalFunctions())
        ui->txtOutput->appendPlainText(QString::fromStdString(sym));
    */

    if (mAda.hasError()) {
        const QString message = QString::fromStdString(mAda.lastError());
        appendOutputLine(ui->txtOutput, QString("Error: %1").arg(message), QColor("#b00020"));
        QMessageBox::critical(this, tr("NeoAda Exception"), message);
    } else if (!unhandledException.isEmpty()) {
        const QString message = tr("Unhandled exception: %1").arg(displayExceptionName(unhandledException));
        appendOutputLine(ui->txtOutput, message, QColor("#b00020"));
        appendOutputLine(ui->txtOutput, tr("Finished in %1 ms").arg(elapsed));
        QMessageBox::critical(this, tr("NeoAda Exception"), message);
    } else {
        if (ret.type() != Nda::Undefined)
            ui->txtOutput->appendPlainText("\nreturn: " + QString::fromStdString(ret.toString()));
        ui->txtOutput->appendPlainText(tr("\nFinished in %1 ms").arg(elapsed));
    }
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::onStop()
{
    if (mRunning)
        return;

    for (auto *scenario : mScenarios)
        scenario->stop();
    ui->txtOutput->clear();
}

//-------------------------------------------------------------------------------------------------
void NeoAdaEdit::closeEvent(QCloseEvent *event)
{
    for (auto *scenario : mScenarios)
        scenario->stop();

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

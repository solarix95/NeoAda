#ifndef NEOADAEDIT_H
#define NEOADAEDIT_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QVector>

namespace Ui {
class NeoAdaEdit;
}

class NdaRuntime;
class NeoAdaHighlighter;
class QComboBox;
class QTextBrowser;
class QLabel;
class QPushButton;
class QCheckBox;
class QSplitter;
class QRadioButton;
class AbstractScenario;

class NeoAdaEdit : public QWidget
{
    Q_OBJECT

public:
    explicit NeoAdaEdit(QWidget *parent = nullptr);
    ~NeoAdaEdit();

    void saveState();
    void restoreState();

private slots:

    // File handling
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();

    // Script handling
    void onPlay();
    void onStop();
    void updateTitle();

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    struct Example {
        QString title;
        QString level;
        QString code;
    };

    Ui::NeoAdaEdit    *ui;
    QString            mCurrentFileName;
    NeoAdaHighlighter *mHighlighter;
    NdaRuntime         &mAda;
    QVector<Example>    mExamples;
    QVector<AbstractScenario*> mScenarios;
    QTextBrowser       *mGuide;
    QSplitter          *mMainSplitter;
    AbstractScenario   *mActiveScenario;
    QPushButton        *mNewButton;
    QCheckBox          *mBufferedOutput;
    QStringList         mRunOutput;
    bool                mRunning;

    void setRunning(bool running);
    bool confirmDiscardChanges();
    bool isDirty() const;   // compare File-Content with QTextEdit "txtScript"
    void loadCurrentFile(); //load mCurrentFileName into QTextEdit "txtScript"
    void initRuntime();
    void initEditor();
    void initLearningTools();
    void initExamples();
    void initScenarios();
    void resetScenarios();
    void activateScenario(const QString &packageName);
    void loadEditorAddon(const std::string &addonName);
    void setScriptText(const QString &text, const QString &fileName = QString());

};

#endif // NEOADAEDIT_H

#ifndef NEOADAEDIT_H
#define NEOADAEDIT_H

#include <QWidget>
#include <QString>

namespace Ui {
class NeoAdaEdit;
}

class NdaRuntime;
class NeoAdaHighlighter;

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
    void onOpen();
    void onSave();
    void onSaveAs();

    // Script handling
    void onPlay();
    void onStop();

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    Ui::NeoAdaEdit    *ui;
    QString            mCurrentFileName;
    NeoAdaHighlighter *mHighlighter;
    NdaRuntime         &mAda;

    bool isDirty() const;   // compare File-Content with QTextEdit "txtScript"
    void loadCurrentFile(); //load mCurrentFileName into QTextEdit "txtScript"
    void initRuntime();
    void initEditor();

};

#endif // NEOADAEDIT_H

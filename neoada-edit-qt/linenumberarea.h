#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H


#include <QTextEdit>
#include <QWidget>
#include <QLabel>

class NeoCodeEdit;

// Links neben dem Editor (Zeilennummern)
class LineNumberArea : public QWidget
{
public:
    LineNumberArea(NeoCodeEdit *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    NeoCodeEdit *mEditor;
};


#endif // LINENUMBERAREA_H

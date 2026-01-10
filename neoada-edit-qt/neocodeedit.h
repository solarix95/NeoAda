#ifndef NEOCODEEDIT_H
#define NEOCODEEDIT_H

#include <QTextEdit>
#include <QLabel>

class LineNumberArea;

class NeoCodeEdit : public QTextEdit
{
public:
    NeoCodeEdit(QWidget *parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();
    void updateCursorOverlay();

private:
    LineNumberArea *mLineNumberArea = nullptr;
    QLabel *mCursorOverlay = nullptr;
};

#endif // NEOCODEEDIT_H

#include "linenumberarea.h"
#include "neocodeedit.h"

LineNumberArea::LineNumberArea(NeoCodeEdit *editor)
    : QWidget(editor), mEditor(editor)
{
}


QSize LineNumberArea::sizeHint() const
{
    return QSize(mEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    mEditor->lineNumberAreaPaintEvent(event);
}

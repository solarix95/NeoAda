#include <QAbstractTextDocumentLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QResizeEvent>
#include <QTextBlock>
#include "neocodeedit.h"
#include "linenumberarea.h"


NeoCodeEdit::NeoCodeEdit(QWidget *parent)
    : QTextEdit(parent)
{
    // Empfehlung für Code: Monospace + NoWrap (optional)
    // setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    setLineWrapMode(QTextEdit::NoWrap);

    // Zeilennummern-Gutter
    mLineNumberArea = new LineNumberArea(this);

    // Cursor-Overlay (unten rechts im Editor)
    mCursorOverlay = new QLabel(this);
    mCursorOverlay->setText("Ln 1, Col 1");
    mCursorOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    mCursorOverlay->setStyleSheet(
        "QLabel {"
        "  padding: 2px 6px;"
        "  border-radius: 6px;"
        "  background: rgba(0,0,0,120);"
        "  color: white;"
        "  font-size: 11px;"
        "}"
        );

    // Ränder links für Zeilennummern reservieren
    updateLineNumberAreaWidth();

    connect(this, &QTextEdit::cursorPositionChanged, this, &NeoCodeEdit::highlightCurrentLine);
    connect(this, &QTextEdit::cursorPositionChanged, this, &NeoCodeEdit::updateCursorOverlay);

    // QTextEdit hat kein updateRequest-Signal wie QPlainTextEdit,
    // aber wir können über Scrollbars + documentLayout reagieren:
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this]{
        mLineNumberArea->update();
        updateCursorOverlay();
    });
    connect(document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged,
            this, [this]{
                updateLineNumberAreaWidth();
                mLineNumberArea->update();
                updateCursorOverlay();
            });

    highlightCurrentLine();
    updateCursorOverlay();
}

int NeoCodeEdit::lineNumberAreaWidth() const
{
    // Anzahl Stellen abhängig von Blockcount
    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    // Abstand + Ziffernbreite
    const int space = 8 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void NeoCodeEdit::updateLineNumberAreaWidth()
{
    // viewport margins: links Platz schaffen
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void NeoCodeEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        mLineNumberArea->scroll(0, dy);
    else
        mLineNumberArea->update(0, rect.y(), mLineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}

void NeoCodeEdit::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);

    // Gutter links positionieren
    const QRect cr = contentsRect();
    mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));

    // Cursor-Overlay unten rechts über dem Inhalt (Viewport)
    updateCursorOverlay();
}


void NeoCodeEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::NoModifier) {
        textCursor().insertText(QStringLiteral("    "));
        event->accept();
        return;
    }

    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
            && (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::KeypadModifier)) {
        QTextCursor cursor = textCursor();
        const QString text = cursor.block().text();
        QString indent;
        for (const QChar ch : text) {
            if (ch == QLatin1Char(' ') || ch == QLatin1Char('\t'))
                indent.append(ch);
            else
                break;
        }

        cursor.beginEditBlock();
        cursor.insertBlock();
        cursor.insertText(indent);
        cursor.endEditBlock();
        setTextCursor(cursor);
        event->accept();
        return;
    }

    QTextEdit::keyPressEvent(event);
}

void NeoCodeEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(mLineNumberArea);
    painter.fillRect(event->rect(), QColor(245, 245, 245)); // Gutter-Hintergrund

    QTextBlock block = document()->firstBlock();
    int blockNumber = 0;

    // Wir müssen y-Positionen anhand des Layouts ermitteln
    const auto *layout = document()->documentLayout();

    // Top of first visible block:
    const int yOffset = verticalScrollBar()->value();

    while (block.isValid()) {
        const QRectF br = layout->blockBoundingRect(block);
        int top = int(br.top()) - yOffset;
        int bottom = top + int(br.height());

        // nur zeichnen, was sichtbar ist
        if (bottom >= event->rect().top() && top <= event->rect().bottom()) {
            const QString number = QString::number(blockNumber + 1);

            // aktuelle Zeile hervorheben (im Gutter)
            bool isCurrent = (textCursor().block() == block);
            painter.setPen(isCurrent ? QColor(20, 20, 20) : QColor(120, 120, 120));

            painter.drawText(0, top, mLineNumberArea->width() - 4, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        ++blockNumber;

        // kleiner Performance-Stop wenn wir weit unterhalb sind
        if (top > event->rect().bottom())
            break;
    }

    // Separator-Linie rechts im Gutter
    painter.setPen(QColor(210, 210, 210));
    painter.drawLine(mLineNumberArea->width() - 1, event->rect().top(),
                     mLineNumberArea->width() - 1, event->rect().bottom());
}

void NeoCodeEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(255, 255, 200)); // sanftes Gelb
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);

        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
    mLineNumberArea->update();
}

void NeoCodeEdit::updateCursorOverlay()
{
    QTextCursor c = textCursor();
    const int line = c.blockNumber() + 1;          // 1-basiert
    const int col  = c.positionInBlock() + 1;      // 1-basiert

    mCursorOverlay->setText(QString("Ln %1, Col %2").arg(line).arg(col));
    mCursorOverlay->adjustSize();

    // Position unten rechts innerhalb des Viewports, mit Innenabstand
    const int pad = 8;
    const QRect vp = viewport()->rect();
    const int x = vp.right() - mCursorOverlay->width() - pad;
    const int y = vp.bottom() - mCursorOverlay->height() - pad;

    // viewport ist Child von QTextEdit intern -> wir setzen relativ zu this:
    // viewport().topLeft() ist i.d.R. (0,0) in QTextEdit, aber sicherheitshalber:
    const QPoint vpTopLeft = viewport()->mapTo(this, QPoint(0, 0));
    mCursorOverlay->move(vpTopLeft.x() + x, vpTopLeft.y() + y);
    mCursorOverlay->raise();
}

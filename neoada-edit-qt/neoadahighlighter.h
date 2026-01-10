#ifndef NEOADAHIGHLIGHTER_H
#define NEOADAHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>
#include <QRegularExpression>

class NeoAdaHighlighter final : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit NeoAdaHighlighter(QTextDocument* document);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct Rule
    {
        QRegularExpression rx;
        QTextCharFormat fmt;
    };

    QVector<Rule> m_rules;

    // Multi-line comments:  /* ... */
    QRegularExpression m_mlCommentStart;
    QRegularExpression m_mlCommentEnd;
    QTextCharFormat m_mlCommentFormat;

    // Single-line comments: -- ...
    QRegularExpression m_slComment;
    QTextCharFormat m_slCommentFormat;

    // Helper
    static QRegularExpression wordRx(const QString& word);
    static QRegularExpression wordsRx(const QStringList& words);
};

#endif // NEOADAHIGHLIGHTER_H

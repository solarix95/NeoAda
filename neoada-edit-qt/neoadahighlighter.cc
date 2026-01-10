#include <QTextDocument>
#include <QStringList>

#include "neoadahighlighter.h"

static QTextCharFormat makeFormat(bool bold = false, bool italic = false)
{
    QTextCharFormat f;
    f.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    f.setFontItalic(italic);
    return f;
}

QRegularExpression NeoAdaHighlighter::wordRx(const QString& word)
{
    // \bword\b with escaping
    return QRegularExpression(QStringLiteral(R"(\b%1\b)").arg(QRegularExpression::escape(word)));
}

QRegularExpression NeoAdaHighlighter::wordsRx(const QStringList& words)
{
    // \b(word1|word2|...)\b
    QStringList escaped;
    escaped.reserve(words.size());
    for (const auto& w : words)
        escaped.push_back(QRegularExpression::escape(w));
    const auto pattern = QStringLiteral(R"(\b(%1)\b)").arg(escaped.join('|'));
    return QRegularExpression(pattern);
}

NeoAdaHighlighter::NeoAdaHighlighter(QTextDocument* document)
    : QSyntaxHighlighter(document)
{
    // ---------- Formats ----------
    auto kwFmt     = makeFormat(true, false);
    auto typeFmt   = makeFormat(true, false);
    auto litFmt    = makeFormat(false, false);
    auto fnFmt     = makeFormat(false, false);
    auto opwFmt    = makeFormat(true, false);
    auto attrFmt   = makeFormat(false, true);

    // Optional: If you want colors, uncomment and pick your palette.
    kwFmt.setForeground(QColor("#6f42c1"));
    typeFmt.setForeground(QColor("#005cc5"));
    litFmt.setForeground(QColor("#d73a49"));
    // fnFmt.setForeground(QColor("#032f62"));
    // opwFmt.setForeground(QColor("#22863a"));
    // attrFmt.setForeground(QColor("#e36209"));

    // ---------- Keywords ----------
    const QStringList keywords = {
        "declare", "type", "is", "of", "list",
        "if", "then", "elsif", "else", "end",
        "for", "in", "while", "loop",
        "procedure", "function", "return", "begin",
        "break", "continue", "when"
    };
    m_rules.push_back({ wordsRx(keywords), kwFmt });

    // ---------- Types (built-ins) ----------
    const QStringList types = {
        "Boolean", "Natural", "Positive", "Number",
        "Character", "String", "Byte", "any"
    };
    m_rules.push_back({ wordsRx(types), typeFmt });

    // ---------- Operator-words ----------
    const QStringList opWords = {
        "and", "or", "not", "mod", "rem"
    };
    m_rules.push_back({ wordsRx(opWords), opwFmt });

    // ---------- Literals ----------
    // true/false as booleans
    m_rules.push_back({ wordsRx({"true", "false"}), litFmt });

    // Numbers (integer/float, with optional exponent)
    // Examples: 42, 3.14, .5, 1e-3, 2.0E+5
    m_rules.push_back({
        QRegularExpression(QStringLiteral(R"((?<![\w.])(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?(?![\w.]))")),
        litFmt
    });

    // Character literal: 'a' or '\n' (simple)
    m_rules.push_back({
        QRegularExpression(QStringLiteral(R"('([^'\\]|\\.)')")),
        litFmt
    });

    // String literal: " ... " with escapes
    m_rules.push_back({
        QRegularExpression(QStringLiteral(R"("([^"\\]|\\.)*")")),
        litFmt
    });

    // ---------- Function call highlighting ----------
    // Name before '('  => foo(...)
    m_rules.push_back({
        QRegularExpression(QStringLiteral(R"(\b([A-Za-z_]\w*)\s*(?=\())")),
        fnFmt
    });

    // Method-style call with dot: x.length(...) or x.length
    // Highlights the member name after '.'
    m_rules.push_back({
        QRegularExpression(QStringLiteral(R"(\.\s*([A-Za-z_]\w*)\b)")),
        attrFmt
    });

    // ---------- Comments ----------
    // Single line: -- ...
    m_slCommentFormat = makeFormat(false, true);
    m_slCommentFormat.setForeground(QColor("#6a737d"));
    m_slComment = QRegularExpression(QStringLiteral(R"(--[^\n]*)"));

    // Multi line: /* ... */
    m_mlCommentFormat = makeFormat(false, true);
    m_mlCommentFormat.setForeground(QColor("#6a737d"));
    m_mlCommentStart = QRegularExpression(QStringLiteral(R"(/\*)"));
    m_mlCommentEnd   = QRegularExpression(QStringLiteral(R"(\*/)"));
}

void NeoAdaHighlighter::highlightBlock(const QString& text)
{
    // 1) Apply normal rules (non-comment)
    for (const auto& r : m_rules)
    {
        auto it = r.rx.globalMatch(text);
        while (it.hasNext())
        {
            const auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), r.fmt);
        }
    }

    // 2) Single-line comments override
    {
        auto it = m_slComment.globalMatch(text);
        while (it.hasNext())
        {
            const auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), m_slCommentFormat);
        }
    }

    // 3) Multi-line comments
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
    {
        auto m = m_mlCommentStart.match(text);
        startIndex = m.hasMatch() ? m.capturedStart() : -1;
    }
    else
    {
        startIndex = 0; // continue from previous block
    }

    while (startIndex >= 0)
    {
        auto endMatch = m_mlCommentEnd.match(text, startIndex);
        int endIndex = endMatch.hasMatch() ? endMatch.capturedStart() : -1;

        int commentLength = 0;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = (endIndex - startIndex) + endMatch.capturedLength();
        }

        setFormat(startIndex, commentLength, m_mlCommentFormat);

        // look for next start after this comment
        auto nextStart = m_mlCommentStart.match(text, startIndex + commentLength);
        startIndex = nextStart.hasMatch() ? nextStart.capturedStart() : -1;
    }
}

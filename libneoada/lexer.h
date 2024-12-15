#ifndef LIB_NEOADA_LEXER_
#define LIB_NEOADA_LEXER_

#include <string>
#include <functional>
#include <vector>

class NadaLexer {
public:
    enum class TokenType {
        Identifier,    // Variable oder Funktionsname
        Keyword,       // Reservierte Wörter wie "declare", "if", "then"
        Number,        // Ganzzahl oder Gleitkommazahl
        String,        // String-Literale
        BooleanLiteral,
        Operator,      // Operatoren wie ":=", "**", "/="
        Separator,     // Satzzeichen wie ";", "(", ")"
        Unknown        // Unerwartete oder nicht erkannte Token
    };

    NadaLexer(int lookAhead = 2);

    // Hauptmethode: Skript analysieren und für jedes Token den Callback aufrufen
    void setScript(const std::string& script);
    bool nextToken();
    bool token(std::string& token, NadaLexer::TokenType &t) const;

    std::string          token(int relativeIndex = 0) const;
    NadaLexer::TokenType tokenType(int relativeIndex = 0) const;

private:
    bool parseNext();

    // Hilfsmethoden für das Parsen
    bool isWhitespace(char c) const;
    bool isIdentifierStart(char c) const;
    bool isIdentifierPart(char c) const;
    bool isDigit(char c) const;

    // character-cursor
    bool        shiftToNext(int step = 1);
    bool        atEnd() const;
    const char &currentChar() const;
    char        nextChar() const;

    std::string mScript;
    size_t      mPos;
    int         mReadAhead;

    std::vector<std::pair<std::string,TokenType>> mTokens;
    int                                           mTokenIdx;
};

#endif

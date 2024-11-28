#ifndef LIB_NEOADA_LEXER_
#define LIB_NEOADA_LEXER_

#include <string>
#include <functional>

class NadaLexer {
public:
    enum class TokenType {
        Identifier,    // Variable oder Funktionsname
        Keyword,       // Reservierte Wörter wie "declare", "if", "then"
        Number,        // Ganzzahl oder Gleitkommazahl
        String,        // String-Literale
        Operator,      // Operatoren wie ":=", "**", "/="
        Separator,     // Satzzeichen wie ";", "(", ")"
        Unknown        // Unerwartete oder nicht erkannte Token
    };

    // Hauptmethode: Skript analysieren und für jedes Token den Callback aufrufen
    bool parse(const std::string& script, std::function<void(const std::string& token, NadaLexer::TokenType t)> callback);

private:
    // Hilfsmethoden für das Parsen
    bool isWhitespace(char c) const;
    bool isIdentifierStart(char c) const;
    bool isIdentifierPart(char c) const;
    bool isDigit(char c) const;
};

#endif

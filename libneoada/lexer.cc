#include "lexer.h"
#include <iostream>
#include <unordered_set>

bool NadaLexer::parse(const std::string& script, std::function<void (const std::string &, NadaLexer::TokenType)> callback) {
    size_t i = 0; // Aktuelle Position im Skript

    while (i < script.size()) {
        char c = script[i];
        char cc = (i + 1 < script.size()) ? script[i + 1] : '\0';

        // Whitespace überspringen
        if (isWhitespace(c)) {
            i++;
            continue;
        }

        if (c == '-' && cc == '-') {
            i += 2; // "--" überspringen
            while (i < script.size() && script[i] != '\n') {
                i++;
            }
            continue;
        }

        // Schlüsselwörter, Identifikatoren oder Literale erkennen
        if (isIdentifierStart(c)) {
            size_t start = i;
            while (i < script.size() && isIdentifierPart(script[i])) {
                i++;
            }

            const std::unordered_set<std::string> reservedWords = {
                "declare", "if", "then", "else", "end", "loop", "exit", "procedure",
                "function", "return", "is", "begin", "not", "and", "or", "mod", "rem", "xor"
            };


            std::string token = script.substr(start, i - start);

            // Prüfe, ob das Token ein reserviertes Wort ist
            if (reservedWords.count(token)) {
                callback(token, TokenType::Keyword); // Als reserviertes Wort zurückgeben
            } else {
                callback(token, TokenType::Identifier); // Als Identifier zurückgeben
            }
            continue;
        }

        // Strings erkennen
        if (c == '"') {
            size_t start = i++;  // Startposition des Strings
            std::string stringLiteral;
            bool isValid = true;

            while (i < script.size()) {
                if (script[i] == '"') {
                    if (i + 1 < script.size() && script[i + 1] == '"') {
                        // Doppelte Anführungszeichen innerhalb des Strings ("" -> ")
                        stringLiteral += '"';
                        i += 2; // Überspringe beide Anführungszeichen
                    } else {
                        // Abschluss des Strings
                        i++;
                        break;
                    }
                } else {
                    // Normaler String-Inhalt
                    stringLiteral += script[i];
                    i++;
                }
            }

            if (i > script.size() || script[start] != '"' || script[i - 1] != '"') {
                std::cerr << "Invalid string literal: " << script.substr(start, i - start) << "\n";
                isValid = false;
            }

            if (isValid) {
                callback(script.substr(start, i - start), TokenType::String); // Gebe den gesamten String-Token zurück
            }
            continue;
        }

        // Zahlen erkennen
        if (isDigit(c)) {
            size_t start = i;
            bool isBased = false;
            bool hasDecimal = false;
            bool hasExponent = false;

            // Schritt 1: Numeral oder Base erkennen
            while (i < script.size() && (isDigit(script[i]) || script[i] == '_')) {
                i++;
            }

            // Prüfe auf Base-Literal (z. B. "16#")
            if (i < script.size() && script[i] == '#') {
                isBased = true;
                i++;
                // Schritt 2: Based Numeral verarbeiten
                while (i < script.size() && (isDigit(script[i]) || (script[i] >= 'A' && script[i] <= 'F') || script[i] == '_')) {
                    i++;
                }

                // Prüfe auf abschließendes "#" für Based Numeral
                if (i >= script.size() || script[i] != '#') {
                    std::cerr << "Invalid based literal: " << script.substr(start, i - start) << "\n";
                    return false;
                }
                i++; // Überspringe abschließendes "#"
            } else {
                // Schritt 3: Dezimalpunkt verarbeiten
                if (i < script.size() && script[i] == '.') {
                    hasDecimal = true;
                    i++;
                    while (i < script.size() && (isDigit(script[i]) || script[i] == '_')) {
                        i++;
                    }
                }
            }

            // Schritt 4: Exponent verarbeiten
            if (i < script.size() && (script[i] == 'E' || script[i] == 'e')) {
                hasExponent = true;
                i++;
                if (i < script.size() && (script[i] == '+' || script[i] == '-')) {
                    i++;
                }
                if (i >= script.size() || !isDigit(script[i])) {
                    std::cerr << "Invalid exponent: " << script.substr(start, i - start) << "\n";
                    return false;
                }
                while (i < script.size() && (isDigit(script[i]) || script[i] == '_')) {
                    i++;
                }
            }

            // Token zurückgeben
            callback(script.substr(start, i - start),TokenType::Number);
            continue;
        }

        // Mehrstellige Operatoren zuerst prüfen
        std::unordered_set<std::string> twoCharOperators = { ":=", "**", "/=", "<=", ">=" };

        if (i + 1 < script.size()) {
            std::string twoCharOp = script.substr(i, 2);
            if (twoCharOperators.count(twoCharOp)) {
                callback(twoCharOp, TokenType::Operator);
                i += 2; // Zwei Zeichen überspringen
                continue;
            }
        }

        // Einfache einstellige Operatoren prüfen
        std::unordered_set<char> singleCharOperators = { '+', '-', '*', '/', '<', '>', '=', '&' };
        if (singleCharOperators.count(c)) {
            callback(std::string(1, c), TokenType::Operator);
            i++;
            continue;
        }

        // Einzelzeichen-Tokens (Operatoren, Separatoren, etc.)
        // Separatoren
        if (c == ';' || c == ':' || c == '(' || c == ')') {
            callback(std::string(1, c), TokenType::Separator);
            i++;
            continue;
        }

        // Fehlerhafte Zeichen
        std::cerr << "Unexpected character: '" << c << "' at position " << i << "\n";
        callback(std::string(1, c), TokenType::Unknown);
        return false;
    }

    return true; // Parsing erfolgreich
}

// Hilfsmethoden
bool NadaLexer::isWhitespace(char c) const {
    return std::isspace(c);
}

bool NadaLexer::isIdentifierStart(char c) const {
    return std::isalpha(c) || c == '_'; // Buchstaben oder Unterstrich
}

bool NadaLexer::isIdentifierPart(char c) const {
    return std::isalnum(c) || c == '_'; // Buchstaben, Ziffern oder Unterstrich
}

bool NadaLexer::isDigit(char c) const {
    return std::isdigit(c);
}

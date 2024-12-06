#include "lexer.h"
#include <iostream>
#include <assert.h>
#include <unordered_set>

//-------------------------------------------------------------------------------------------------
NadaLexer::NadaLexer()
    : mPos(-1)
    , mReadAhead(0)
    , mTokenIdx(-1)
{
}

//-------------------------------------------------------------------------------------------------
void NadaLexer::setLookAhead(int lookAhead)
{
    assert(lookAhead >= 0);
    mReadAhead = lookAhead;
}

//-------------------------------------------------------------------------------------------------
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

            // Schritt 1: Numeral oder Base erkennen
            while (i < script.size() && (isDigit(script[i]) || script[i] == '_')) {
                i++;
            }

            // Prüfe auf Base-Literal (z. B. "16#")
            if (i < script.size() && script[i] == '#') {
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
                    i++;
                    while (i < script.size() && (isDigit(script[i]) || script[i] == '_')) {
                        i++;
                    }
                }
            }

            // Schritt 4: Exponent verarbeiten
            if (i < script.size() && (script[i] == 'E' || script[i] == 'e')) {
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

//-------------------------------------------------------------------------------------------------
void NadaLexer::setScript(const std::string &script)
{
    mScript = script;
    mPos    = -1;

    mTokens.clear();
    mTokenIdx = -1;
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::nextToken()
{
    while (parseNext()) {
        int availableTokens = (mTokens.size()-1) - mTokenIdx;
        if (availableTokens > mReadAhead)
            break;
    }
    if (mTokenIdx >= (int)mTokens.size())
        return false;


    auto ret =  ++mTokenIdx < (int)mTokens.size();
    std::cout << "TOKEN NOW: " << token() << std::endl;
    return ret;
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::nextToken(std::string &tstring, TokenType &ttype)
{
    tstring = "";
    ttype   = NadaLexer::TokenType::Unknown;

    if (!nextToken())
        return false;

    return token(tstring,ttype);
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::token(std::string &token, TokenType &t) const
{
    if (mTokens.empty() || (mTokenIdx >= (int)mTokens.size()))
        return false;

    token = mTokens[mTokenIdx].first;
    t     = mTokens[mTokenIdx].second;

    return true;
}

//-------------------------------------------------------------------------------------------------
std::string NadaLexer::token(int relativeIndex) const
{
    int absoluteIndex = mTokenIdx + relativeIndex;
    if (mTokens.empty() || (absoluteIndex < 0) || (absoluteIndex >= (int)mTokens.size()))
        return std::string();

    return mTokens[absoluteIndex].first;
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::parseNext()
{
    mPos++;
    while (mPos < mScript.size()) {
        char c = mScript[mPos];
        char cc = (mPos + 1 < mScript.size()) ? mScript[mPos + 1] : '\0';

        // Whitespace überspringen
        if (isWhitespace(c)) {
            mPos++;
            continue;
        }

        if (c == '-' && cc == '-') {
            mPos += 2; // "--" überspringen
            while (mPos < mScript.size() && mScript[mPos] != '\n') {
                mPos++;
            }
            continue;
        }

        // Schlüsselwörter, Identifikatoren oder Literale erkennen
        if (isIdentifierStart(c)) {
            size_t start = mPos;
            while (mPos < mScript.size() && isIdentifierPart(mScript[mPos])) {
                mPos++;
            }

            const std::unordered_set<std::string> reservedWords = {
                "declare", "if", "then", "else", "end", "loop", "exit", "procedure",
                "function", "return", "is", "begin", "not", "and", "or", "mod", "rem", "xor"
            };

            std::string token = mScript.substr(start, mPos - start);
            mPos--; // 1 Character zuviel eingelesen..

            // Prüfe, ob das Token ein reserviertes Wort ist
            if (reservedWords.count(token)) {
                mTokens.push_back(std::make_pair(token,TokenType::Keyword));
                return true;
            } else {
                mTokens.push_back(std::make_pair(token,TokenType::Identifier));
                return true;
            }
            continue;
        }

        // Strings erkennen
        if (c == '"') {
            size_t start = mPos++;  // Startposition des Strings
            std::string stringLiteral;
            bool isValid = true;

            while (mPos < mScript.size()) {
                if (mScript[mPos] == '"') {
                    if (mPos + 1 < mScript.size() && mScript[mPos + 1] == '"') {
                        // Doppelte Anführungszeichen innerhalb des Strings ("" -> ")
                        stringLiteral += '"';
                        mPos += 2; // Überspringe beide Anführungszeichen
                    } else {
                        // Abschluss des Strings
                        break;
                    }
                } else {
                    // Normaler String-Inhalt
                    stringLiteral += mScript[mPos];
                    mPos++;
                }
            }

            if (mPos > mScript.size() || mScript[start] != '"' || mScript[mPos] != '"') {
                std::cerr << "Invalid string literal: " << mScript.substr(start, mPos + 1 - start) << "\n";
                isValid = false;
            }

            if (isValid) {
                mTokens.push_back(std::make_pair(mScript.substr(start, mPos - start + 1),TokenType::String));
                return true;
            }
            continue;
        }

        // Zahlen erkennen
        if (isDigit(c)) {
            size_t start = mPos;

            // Schritt 1: Numeral oder Base erkennen
            while (mPos < mScript.size() && (isDigit(mScript[mPos]) || mScript[mPos] == '_')) {
                mPos++;
            }

            // Prüfe auf Base-Literal (z. B. "16#")
            if (mPos < mScript.size() && mScript[mPos] == '#') {
                mPos++;
                // Schritt 2: Based Numeral verarbeiten
                while (mPos < mScript.size() && (isDigit(mScript[mPos]) || (mScript[mPos] >= 'A' && mScript[mPos] <= 'F') || mScript[mPos] == '_')) {
                    mPos++;
                }

                // Prüfe auf abschließendes "#" für Based Numeral
                if (mPos >= mScript.size() || mScript[mPos] != '#') {
                    std::cerr << "Invalid based literal: " << mScript.substr(start, mPos - start) << "\n";
                    return false;
                }
                mPos++; // Überspringe abschließendes "#"
            } else {
                // Schritt 3: Dezimalpunkt verarbeiten
                if (mPos < mScript.size() && mScript[mPos] == '.') {
                    mPos++;
                    while (mPos < mScript.size() && (isDigit(mScript[mPos]) || mScript[mPos] == '_')) {
                        mPos++;
                    }
                }
            }

            // Schritt 4: Exponent verarbeiten
            if (mPos < mScript.size() && (mScript[mPos] == 'E' || mScript[mPos] == 'e')) {
                mPos++;
                if (mPos < mScript.size() && (mScript[mPos] == '+' || mScript[mPos] == '-')) {
                    mPos++;
                }
                if (mPos >= mScript.size() || !isDigit(mScript[mPos])) {
                    std::cerr << "Invalid exponent: " << mScript.substr(start, mPos - start) << "\n";
                    return false;
                }
                while (mPos < mScript.size() && (isDigit(mScript[mPos]) || mScript[mPos] == '_')) {
                    mPos++;
                }
            }

            // Token zurückgeben
            mTokens.push_back(std::make_pair(mScript.substr(start, mPos - start),TokenType::Number));
            mPos--;
            return true;
        }

        // Mehrstellige Operatoren zuerst prüfen
        std::unordered_set<std::string> twoCharOperators = { ":=", "**", "/=", "<=", ">=" };

        if (mPos + 1 < mScript.size()) {
            std::string twoCharOp = mScript.substr(mPos, 2);
            if (twoCharOperators.count(twoCharOp)) {
                mTokens.push_back(std::make_pair(twoCharOp,TokenType::Operator));
                mPos++;
                return true;
            }
        }

        // Einfache einstellige Operatoren prüfen
        std::unordered_set<char> singleCharOperators = { '+', '-', '*', '/', '<', '>', '=', '&' };
        if (singleCharOperators.count(c)) {
            mTokens.push_back(std::make_pair(std::string(1, c),TokenType::Operator));
            return true;
        }

        // Einzelzeichen-Tokens (Operatoren, Separatoren, etc.)
        // Separatoren
        if (c == ';' || c == ':' || c == '(' || c == ')') {
            mTokens.push_back(std::make_pair(std::string(1, c),TokenType::Separator));
            return true;
        }

        // Fehlerhafte Zeichen
        std::cerr << "Unexpected character: '" << c << "' at position " << mPos << "\n";
        return false;
    }

    return false;
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

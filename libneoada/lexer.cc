#include "lexer.h"
#include <iostream>
#include <assert.h>
#include <unordered_set>

#include "utils.h"

//-------------------------------------------------------------------------------------------------
NadaLexer::NadaLexer()
    : mPos(-1)
    , mReadAhead(2)
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

    // std::cout << "TOKEN " << mTokens[absoluteIndex].first << std::endl;
    return mTokens[absoluteIndex].first;
}

//-------------------------------------------------------------------------------------------------
NadaLexer::TokenType NadaLexer::tokenType(int relativeIndex) const
{
    int absoluteIndex = mTokenIdx + relativeIndex;
    if (mTokens.empty() || (absoluteIndex < 0) || (absoluteIndex >= (int)mTokens.size()))
        return NadaLexer::TokenType::Unknown;

    return mTokens[absoluteIndex].second;
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

            std::string token      = mScript.substr(start, mPos - start);
            std::string lowerToken = Nada::toLower(token);
            mPos--; // 1 Character zuviel eingelesen..

            const std::unordered_set<std::string> reservedWords = {
                "declare", "if", "then", "else", "elsif", "end", "while", "loop", "exit", "procedure",
                "function", "return", "is", "begin", "not", "and", "or", "mod", "rem", "xor"
            };

            const std::unordered_set<std::string> booleanLiteral = {
                "true", "false"
            };

            // Prüfe, ob das Token ein reserviertes Wort ist
            if (reservedWords.count(lowerToken)) {
                mTokens.push_back(std::make_pair(lowerToken,TokenType::Keyword));
                return true;
            } else if (booleanLiteral.count(lowerToken)) {
                mTokens.push_back(std::make_pair(lowerToken,TokenType::BooleanLiteral));
                return true;
            }else {
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
                mTokens.push_back(std::make_pair(stringLiteral,TokenType::String));
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

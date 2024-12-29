#include "lexer.h"
#include <iostream>
#include <assert.h>
#include <unordered_set>

#include "exception.h"
#include "private/utils.h"

//-------------------------------------------------------------------------------------------------
NadaLexer::NadaLexer(int lookAhead)
    : mPos(-1)
    , mReadAhead(lookAhead)
    , mTokenIdx(-1)
{
    assert(mReadAhead >= 0);
}

//-------------------------------------------------------------------------------------------------
void NadaLexer::setScript(const std::string &script)
{
    mScript = script;
    mPos    = -1;

    mTokens.clear();
    mTokenIdx = -1;

    mRow    = 1;
    mColumn = 1;
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
bool NadaLexer::token(std::string &token, TokenType &t) const
{
    if (mTokens.empty() || (mTokenIdx >= (int)mTokens.size()))
        return false;

    token = mTokens[mTokenIdx].value;
    t     = mTokens[mTokenIdx].type;

    return true;
}

//-------------------------------------------------------------------------------------------------
std::string NadaLexer::token(int relativeIndex) const
{
    int absoluteIndex = mTokenIdx + relativeIndex;
    if (mTokens.empty() || (absoluteIndex < 0) || (absoluteIndex >= (int)mTokens.size()))
        return std::string();

    // std::cout << "TOKEN " << mTokens[absoluteIndex].value << " " << positionToText(relativeIndex) << std::endl;
    return mTokens[absoluteIndex].value;
}

//-------------------------------------------------------------------------------------------------
NadaLexer::TokenType NadaLexer::tokenType(int relativeIndex) const
{
    int absoluteIndex = mTokenIdx + relativeIndex;
    if (mTokens.empty() || (absoluteIndex < 0) || (absoluteIndex >= (int)mTokens.size()))
        return NadaLexer::TokenType::Unknown;

    return mTokens[absoluteIndex].type;
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::tokenPosition(int &row, int &column, int relativeIndex) const
{
    row    = 0;
    column = 0;

    int absoluteIndex = mTokenIdx + relativeIndex;
    if (mTokens.empty() || (absoluteIndex < 0) || (absoluteIndex >= (int)mTokens.size()))
        return false;

    row    = mTokens[absoluteIndex].row;
    column = mTokens[absoluteIndex].column;

    return true;
}

//-------------------------------------------------------------------------------------------------
std::string NadaLexer::positionToText(int relativeIndex) const
{
    int r, c;
    if (!tokenPosition(r,c,relativeIndex))
        return "";

    return "at " + std::to_string(r) + "/" + std::to_string(c);
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::tokenIsValid() const
{
    return mTokenIdx >= 0 && mTokenIdx < mTokens.size();
}

//-------------------------------------------------------------------------------------------------
int NadaLexer::line() const
{
    return tokenIsValid() ? mTokens[mTokenIdx].row : mRow;
}

//-------------------------------------------------------------------------------------------------
int NadaLexer::column() const
{
    return tokenIsValid() ? mTokens[mTokenIdx].column : mColumn;
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::parseNext()
{
    mPos++;
    while (!atEnd()) {

        // Whitespace überspringen
        if (isWhitespace(currentChar())) {
            shiftToNext();
            continue;
        }

        if (currentChar() == '-' && nextChar() == '-') {
            shiftToNext(2); // "--" überspringen
            while (!atEnd() && currentChar() != '\n') {
                shiftToNext();
            }
            continue;
        }

        // Schlüsselwörter, Identifikatoren oder Literale erkennen
        if (isIdentifierStart(currentChar())) {
            size_t start = mPos;
            while (!atEnd() && isIdentifierPart(currentChar())) {
                shiftToNext();
            }

            std::string token      = mScript.substr(start, mPos - start);
            std::string lowerToken = Nada::toLower(token);
            shiftToNext(-1); // 1 Character zuviel eingelesen..

            static const std::unordered_set<std::string> reservedWords = {
                "declare", "volatile",
                "if", "then", "else", "elsif", "end",
                "while", "loop", "break", "continue", "when",
                "for", "in", "out", "reverse",
                "procedure", "function", "return", "is", "begin", "not", "and", "or", "mod", "rem", "xor"
            };

            static const std::unordered_set<std::string> booleanLiteral = {
                "true", "false"
            };

            // Prüfe, ob das Token ein reserviertes Wort ist
            if (reservedWords.count(lowerToken)) {
                mTokens.push_back(Token(lowerToken,TokenType::Keyword, mRow, mColumn));
                return true;
            } else if (booleanLiteral.count(lowerToken)) {
                mTokens.push_back(Token(lowerToken,TokenType::BooleanLiteral, mRow,mColumn));
                return true;
            }else {
                mTokens.push_back(Token(token,TokenType::Identifier, mRow,mColumn));
                return true;
            }
            continue;
        }

        // Strings erkennen
        if (currentChar() == '"') {
            size_t start = mPos;  // Startposition des Strings
            shiftToNext();
            std::string stringLiteral;
            bool isValid = true;

            while (!atEnd()) {
                if (currentChar() == '"') {
                    if (nextChar() == '"') {
                        // Doppelte Anführungszeichen innerhalb des Strings ("" -> ")
                        stringLiteral += '"';
                        shiftToNext(2); // Überspringe beide Anführungszeichen
                    } else {
                        // Abschluss des Strings
                        break;
                    }
                } else {
                    // Normaler String-Inhalt
                    stringLiteral += currentChar();
                    shiftToNext();
                }
            }

            if (atEnd() || mScript[start] != '"' || currentChar() != '"') {
                if (atEnd())
                    throw NadaException(Nada::Error::UnexpectedEof, mRow, mColumn);
                else
                    throw NadaException(Nada::Error::InvalidStringLiteral, mRow, mColumn);

                // std::cerr << "Invalid string literal: " << mScript.substr(start, mPos + 1 - start) << "\n";
                isValid = false;
            }

            if (isValid) {
                mTokens.push_back(Token(stringLiteral,TokenType::String, mColumn, mRow));
                return true;
            }
            continue;
        }

        // Zahlen erkennen
        if (isDigit(currentChar())) {
            size_t start = mPos;

            // Schritt 1: Numeral oder Base erkennen
            while (!atEnd() && (isDigit(currentChar()) || currentChar() == '_')) {
                shiftToNext();
            }

            // Prüfe auf Base-Literal (z. B. "16#")
            if (!atEnd() && currentChar() == '#') {
                shiftToNext();
                // Schritt 2: Based Numeral verarbeiten
                while (!atEnd() && (isDigit(currentChar()) || (currentChar() >= 'A' && currentChar() <= 'F') || currentChar() == '_')) {
                    shiftToNext();
                }

                // Prüfe auf abschließendes "#" für Based Numeral
                if (atEnd())
                    throw NadaException(Nada::Error::UnexpectedEof, mRow, mColumn);
                if (currentChar() != '#')
                    throw NadaException(Nada::Error::InvalidBasedLiteral, mRow, mColumn, mScript.substr(start, mPos - start));
                shiftToNext(); // Überspringe abschließendes "#"
            } else {
                // Schritt 3: Dezimalpunkt verarbeiten
                if (!atEnd() && currentChar() == '.' && nextChar() != '.' /* Range Operator */) {
                    shiftToNext();
                    while (!atEnd() && (isDigit(currentChar()) || currentChar() == '_')) {
                        shiftToNext();
                    }
                }
            }

            // Schritt 4: Exponent verarbeiten
            if (!atEnd() && (currentChar() == 'E' || currentChar() == 'e')) {
                shiftToNext();
                if (!atEnd() && (currentChar() == '+' || currentChar() == '-')) {
                    shiftToNext();
                }
                if (atEnd())
                    throw NadaException(Nada::Error::UnexpectedEof, mRow, mColumn);

                if (!isDigit(currentChar()))
                    // std::cerr << "Invalid exponent: " << mScript.substr(start, mPos - start) << "\n";
                    throw NadaException(Nada::Error::InvalidExponent, mRow, mColumn, mScript.substr(start, mPos - start));
                while (!atEnd() && (isDigit(currentChar()) || currentChar() == '_')) {
                    shiftToNext();
                }
            }

            // Token zurückgeben
            mTokens.push_back(Token(mScript.substr(start, mPos - start),TokenType::Number, mRow, mColumn));
            shiftToNext(-1);
            return true;
        }

        // Mehrstellige Operatoren zuerst prüfen
        static const std::unordered_set<std::string> twoCharOperators = { ":=", "**", "/=", "<=", ">=", ".." };

        if (nextChar() != '\0') {
            std::string twoCharOp = mScript.substr(mPos, 2);
            if (twoCharOperators.count(twoCharOp)) {
                mTokens.push_back(Token(twoCharOp,TokenType::Operator, mRow, mColumn));
                shiftToNext();
                return true;
            }
        }

        // Einfache einstellige Operatoren prüfen
        static const std::unordered_set<char> singleCharOperators = { '+', '-', '*', '/', '<', '>', '=', '&' };
        if (singleCharOperators.count(currentChar())) {
            mTokens.push_back(Token(std::string(1, currentChar()),TokenType::Operator, mRow, mColumn));
            return true;
        }

        // Einzelzeichen-Tokens (Operatoren, Separatoren, etc.)
        // Separatoren
        if (currentChar() == ';' || currentChar() == ',' || currentChar() == '.' || currentChar() == ':' || currentChar() == '(' || currentChar() == ')') {
            mTokens.push_back(Token(std::string(1, currentChar()),TokenType::Separator, mRow, mColumn));
            return true;
        }

        // Fehlerhafte Zeichen
        throw NadaException(Nada::Error::InvalidCharacter, mRow, mColumn, std::string(1, currentChar()));
        // std::cerr << "Unexpected character: '" << currentChar() << "' at position " << mPos << "\n";
        return false;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::isWhitespace(char c) const {
    return std::isspace(c);
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::isIdentifierStart(char c) const {
    return std::isalpha(c) || c == '_'; // Buchstaben oder Unterstrich
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::isIdentifierPart(char c) const {
    return std::isalnum(c) || c == '_'; // Buchstaben, Ziffern oder Unterstrich
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::isDigit(char c) const {
    return std::isdigit(c);
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::shiftToNext(int step)
{
    assert(step != 0);

    if (atEnd())
        return false;
    mPos += step;

    if (mPos >= 0 && mPos < mScript.length() && mScript[mPos] == '\n') {
        mRow += step > 0 ? +1 : -1;
        mColumn = 0;
    } else if (!atEnd())
        mColumn++;

    return atEnd();
}

//-------------------------------------------------------------------------------------------------
bool NadaLexer::atEnd() const
{
    return mPos >= mScript.length();
}

//-------------------------------------------------------------------------------------------------
const char &NadaLexer::currentChar() const
{
    assert(!atEnd());
    return mScript[mPos];
}

//-------------------------------------------------------------------------------------------------
char NadaLexer::nextChar() const
{
    return (mPos + 1 < mScript.size()) ? mScript[mPos + 1] : '\0';
}

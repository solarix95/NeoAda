#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

namespace Nada {
enum Error {
     NoError = 0,

    // Static Erros
    // ----------------------------------------------
    // Lexer
    InvalidCharacter,
    InvalidStringLiteral,
    InvalidExponent,
    InvalidBasedLiteral,
    UnexpectedEof,
    UnexpectedStructure,  // assert?


    // Parser
    InvalidStatement,
    IdentifierExpected,
    KeywordExpected,
    InvalidToken,
    InvalidRangeOrIterable,
    UnexpectedClosure,

    // Runtime Exceptions
    // ----------------------------------------------
    AssignmentError,
    IllegalComparison,
    OperatorTypeError,
};
}

class NadaException : public std::exception
{
public:
    NadaException();
    NadaException(Nada::Error code, int line, int row, const std::string &extraInfo = "");

    const char* what() const noexcept override;

    inline Nada::Error code()    const { return mCode;   }
    inline int         line()    const { return mLine;   }
    inline int         column()  const { return mColumn; }

private:
    std::string messageByCode() const;
    std::string extraMsg() const;

    Nada::Error mCode;
    int         mLine;
    int         mColumn;
    std::string mMessage;
    std::string mExtra;
};

#endif // EXCEPTION_H

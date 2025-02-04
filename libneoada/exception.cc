#include "exception.h"

//-------------------------------------------------------------------------------------------------
NdaException::NdaException()
    : mCode(Nada::Error::NoError)
    , mLine(0)
    , mColumn(0)
{
}

//-------------------------------------------------------------------------------------------------
NdaException::NdaException(Nada::Error code, int line, int column, const std::string &extraInfo)
    : mCode(code)
    , mLine(line)
    , mColumn(column)
    , mExtra(extraInfo)
{
    if (mCode == Nada::Error::NoError) {
        mMessage = "No Error";
    } else {
        mMessage = "Error: " + messageByCode() + extraMsg() + " at line " + std::to_string(mLine) + ", column " + std::to_string(mColumn);
    }
}

//-------------------------------------------------------------------------------------------------
const char *NdaException::what() const noexcept
{
    return mMessage.c_str();
}

//-------------------------------------------------------------------------------------------------
std::string NdaException::messageByCode() const
{
    switch (mCode) {
    case Nada::Error::NoError: return "";
    case Nada::Error::InvalidStatement:       return "Invalid statement";
    case Nada::Error::InvalidCharacter:       return "Invalid character";
    case Nada::Error::InvalidStringLiteral:   return "Invalid string literal";
    case Nada::Error::InvalidBasedLiteral:    return "Invalid numeric literal";
    case Nada::Error::InvalidExponent:        return "Invalid numeric exponent";
    case Nada::Error::UnexpectedEof:          return "Unexpected end-of-file";
    case Nada::Error::UnexpectedStructure:    return "Invalid code structure";  // probably an assert..
    case Nada::Error::IdentifierExpected:     return "Identifier expected";
    case Nada::Error::KeywordExpected:        return "Keyword expected";
    case Nada::Error::InvalidToken:           return "Invalid token";
    case Nada::Error::InvalidRangeOrIterable: return "Invalid Range or Iterable";
    case Nada::Error::UnexpectedClosure:      return "Unexpected closure";
    case Nada::Error::AssignmentError:        return "Incompatible datatype";
    case Nada::Error::IllegalComparison:      return "Illegal comparison";
    case Nada::Error::OperatorTypeError:      return "Unsupported operand type(s)";
    case Nada::Error::InvalidNumericValue:    return "Invalid numeric value";
    case Nada::Error::UnknownSymbol:          return "Unknown symbol";

    }
    return "";
}

//-------------------------------------------------------------------------------------------------
std::string NdaException::extraMsg() const
{
    if (mExtra.empty())
        return "";
    return " ('" + mExtra + "') ";
}

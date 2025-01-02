#ifndef NUMERICPARSER_H
#define NUMERICPARSER_H

#include <cstdint>
#include <string>


/*
    Ada95 Numeric Literal Parser

    numeric_literal ::= decimal_literal | based_literal
    decimal_literal ::= numeral [ . numeral ] [ exponent ]
    numeral ::= digit { digit | "_" }
    exponent ::= "E" [ "+" | "-" ] numeral
    based_literal ::= base "#" based_numeral "#" [ exponent ]
    base ::= numeral
    based_numeral ::= extended_digit { extended_digit | "_" }
    extended_digit ::= digit | "A" | "B" | "C" | "D" | "E" | "F"
*/


namespace NadaNumericParser {

std::string removeSeparators(const std::string& str);          // 200_042.23
bool        isFloatingPointLiteral(const std::string& str);    // 5.5?
bool        isBasedLiteral(const std::string& str);            // 16#1F# ?
int64_t     parseBasedLiteral(const std::string& literal, bool &ok);
uint64_t    parseUBasedLiteral(const std::string& literal, bool &ok);
}


#endif // NUMERICPARSER_H

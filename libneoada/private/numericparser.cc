#include "numericparser.h"
#include <cmath>
#include <regex>



std::string NadaNumericParser::removeSeparators(const std::string &str)
{
    std::string cleanStr;
    for (char c : str) {
        if (std::isdigit(c) || (c == '.') || (c == '#')) cleanStr += c; // 10_000_b -> 10000
    }

    // TODO.. was ist schneller?
    /*
        std::copy_if(str.begin(), str.end(), std::back_inserter(cleanStr),
                 [](char c) { return c != '_'; });
    */
    return cleanStr;
}

bool NadaNumericParser::isFloatingPointLiteral(const std::string &str)
{
    if (str.length() <= 1)
        return false;

    static const std::regex floatRegex(R"(^\d+\.\d*([eE][-+]?\d+)?$|^\d+[eE][-+]?\d+$)");
    return std::regex_match(str, floatRegex);
}

bool NadaNumericParser::isBasedLiteral(const std::string &str)
{
    if (str.length() <= 1)
        return false;
    if (str.find('#') == std::string::npos)
        return false;

    static const std::regex basedRegex(R"(^\d+#([0-9A-Fa-f_]+)#([eE][-+]?\d+)?$)");
    return std::regex_match(str, basedRegex);
}

int64_t NadaNumericParser::parseBasedLiteral(const std::string &literal, bool &ok)
{
    ok = false;
    static const std::regex basedRegex(R"(^(\d+)#([0-9A-Fa-f_]+)#$)");
    std::smatch match;
    if (!std::regex_match(literal, match, basedRegex))
        return 0;

    std::string baseStr = match[1];
    std::string numberStr = removeSeparators(match[2]);

    int base = std::stoi(baseStr);
    if (base < 2 || base > 16)
        return 0;

    int64_t value = std::stoll(numberStr, nullptr, base);

    ok = true;
    return value;
}

uint64_t NadaNumericParser::parseUBasedLiteral(const std::string &literal, bool &ok)
{
    ok = false;
    static const std::regex basedRegex(R"(^(\d+)#([0-9A-Fa-f_]+)#$)");
    std::smatch match;
    if (!std::regex_match(literal, match, basedRegex))
        return 0;

    std::string baseStr = match[1];
    std::string numberStr = removeSeparators(match[2]);

    int base = std::stoi(baseStr);
    if (base < 2 || base > 16)
        return 0;

    uint64_t value = std::stoull(numberStr, nullptr, base);

    ok = true;
    return value;
}

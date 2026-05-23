#include "AdaJson.h"
#include "AdaIoFile.h"
#include "../state.h"

#include <cassert>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>

namespace {

NdaVariant stringValue(NdaState *state, const std::string &value)
{
    NdaVariant ret;
    ret.fromString(state->stringType(), value);
    return ret;
}

NdaVariant boolValue(NdaState *state, bool value)
{
    NdaVariant ret;
    ret.fromBool(state->booleanType(), value);
    return ret;
}

NdaVariant naturalValue(NdaState *state, int64_t value)
{
    NdaVariant ret;
    ret.fromNatural(state->naturalType(), value);
    return ret;
}

NdaVariant numberValue(NdaState *state, double value)
{
    NdaVariant ret;
    ret.fromNumber(state->typeByName("number"), value);
    return ret;
}

NdaVariant nullValue(NdaState *state)
{
    NdaVariant ret;
    ret.initType(state->typeByName("any"));
    return ret;
}

void appendHex4(std::ostringstream &out, unsigned char c)
{
    const char *hex = "0123456789abcdef";
    out << "\\u00" << hex[(c >> 4) & 0x0f] << hex[c & 0x0f];
}

std::string escapeJsonString(const std::string &value)
{
    std::ostringstream out;
    out << '"';
    for (unsigned char c : value) {
        switch (c) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (c < 0x20)
                appendHex4(out, c);
            else
                out << static_cast<char>(c);
            break;
        }
    }
    out << '"';
    return out.str();
}

std::string serializeJson(const NdaVariant &value)
{
    switch (value.type()) {
    case Nda::Undefined:
    case Nda::Any:
        return "null";
    case Nda::Boolean:
        return value.toBool() ? "true" : "false";
    case Nda::String:
        return escapeJsonString(value.toString());
    case Nda::Number: {
        const double number = value.toDouble();
        if (!std::isfinite(number))
            return "null";
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out << std::setprecision(17) << number;
        return out.str();
    }
    case Nda::Natural:
    case Nda::Supernatural:
    case Nda::Byte:
        return value.toString();
    case Nda::List: {
        std::string ret = "[";
        for (int i = 0; i < value.listSize(); ++i) {
            if (i > 0)
                ret += ",";
            ret += serializeJson(value.readAccess(i));
        }
        ret += "]";
        return ret;
    }
    case Nda::Dict: {
        std::string ret = "{";
        bool first = true;
        for (const auto &item : value.dictItems()) {
            if (!first)
                ret += ",";
            first = false;
            ret += escapeJsonString(item.first.toString());
            ret += ":";
            ret += serializeJson(item.second);
        }
        ret += "}";
        return ret;
    }
    case Nda::Reference: {
        NdaVariant copy = value;
        copy.dereference();
        return serializeJson(copy);
    }
    default:
        return "null";
    }
}

class JsonParser
{
public:
    JsonParser(NdaState *state, const std::string &text)
        : mState(state), mText(text), mPos(0)
    {}

    bool parse(NdaVariant &ret)
    {
        skipWs();
        if (!parseValue(ret))
            return false;
        skipWs();
        return mPos == mText.size();
    }

private:
    void skipWs()
    {
        while (mPos < mText.size() && std::isspace(static_cast<unsigned char>(mText[mPos])))
            ++mPos;
    }

    bool consume(char c)
    {
        skipWs();
        if (mPos >= mText.size() || mText[mPos] != c)
            return false;
        ++mPos;
        return true;
    }

    bool consumeWord(const std::string &word)
    {
        skipWs();
        if (mText.compare(mPos, word.size(), word) != 0)
            return false;
        mPos += word.size();
        return true;
    }

    bool parseValue(NdaVariant &ret)
    {
        skipWs();
        if (mPos >= mText.size())
            return false;

        const char c = mText[mPos];
        if (c == '"') {
            std::string value;
            if (!parseString(value))
                return false;
            ret = stringValue(mState, value);
            return true;
        }
        if (c == '{')
            return parseObject(ret);
        if (c == '[')
            return parseArray(ret);
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c)))
            return parseNumber(ret);
        if (consumeWord("true")) {
            ret = boolValue(mState, true);
            return true;
        }
        if (consumeWord("false")) {
            ret = boolValue(mState, false);
            return true;
        }
        if (consumeWord("null")) {
            ret = nullValue(mState);
            return true;
        }
        return false;
    }

    bool parseObject(NdaVariant &ret)
    {
        if (!consume('{'))
            return false;
        ret.initType(mState->dictType());
        skipWs();
        if (consume('}'))
            return true;

        while (true) {
            std::string key;
            if (!parseString(key))
                return false;
            if (!consume(':'))
                return false;
            NdaVariant value;
            if (!parseValue(value))
                return false;
            ret.appendToDict(stringValue(mState, key), value);
            skipWs();
            if (consume('}'))
                return true;
            if (!consume(','))
                return false;
        }
    }

    bool parseArray(NdaVariant &ret)
    {
        if (!consume('['))
            return false;
        ret.initType(mState->listType());
        skipWs();
        if (consume(']'))
            return true;

        while (true) {
            NdaVariant value;
            if (!parseValue(value))
                return false;
            ret.appendToList(value);
            skipWs();
            if (consume(']'))
                return true;
            if (!consume(','))
                return false;
        }
    }

    bool parseString(std::string &ret)
    {
        skipWs();
        if (mPos >= mText.size() || mText[mPos] != '"')
            return false;
        ++mPos;
        ret.clear();

        while (mPos < mText.size()) {
            unsigned char c = static_cast<unsigned char>(mText[mPos++]);
            if (c == '"')
                return true;
            if (c == '\\') {
                if (mPos >= mText.size())
                    return false;
                char esc = mText[mPos++];
                switch (esc) {
                case '"': ret.push_back('"'); break;
                case '\\': ret.push_back('\\'); break;
                case '/': ret.push_back('/'); break;
                case 'b': ret.push_back('\b'); break;
                case 'f': ret.push_back('\f'); break;
                case 'n': ret.push_back('\n'); break;
                case 'r': ret.push_back('\r'); break;
                case 't': ret.push_back('\t'); break;
                case 'u': {
                    int codepoint = 0;
                    for (int i = 0; i < 4; ++i) {
                        if (mPos >= mText.size() || !std::isxdigit(static_cast<unsigned char>(mText[mPos])))
                            return false;
                        char h = mText[mPos++];
                        codepoint <<= 4;
                        if (h >= '0' && h <= '9') codepoint += h - '0';
                        else if (h >= 'a' && h <= 'f') codepoint += h - 'a' + 10;
                        else if (h >= 'A' && h <= 'F') codepoint += h - 'A' + 10;
                    }
                    appendUtf8(ret, codepoint);
                    break;
                }
                default:
                    return false;
                }
            } else {
                if (c < 0x20)
                    return false;
                ret.push_back(static_cast<char>(c));
            }
        }
        return false;
    }

    static void appendUtf8(std::string &out, int codepoint)
    {
        if (codepoint <= 0x7f) {
            out.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7ff) {
            out.push_back(static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else {
            out.push_back(static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
            out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        }
    }

    bool parseNumber(NdaVariant &ret)
    {
        skipWs();
        const size_t start = mPos;
        if (mPos < mText.size() && mText[mPos] == '-')
            ++mPos;
        if (mPos >= mText.size())
            return false;
        if (mText[mPos] == '0') {
            ++mPos;
        } else if (std::isdigit(static_cast<unsigned char>(mText[mPos]))) {
            while (mPos < mText.size() && std::isdigit(static_cast<unsigned char>(mText[mPos])))
                ++mPos;
        } else {
            return false;
        }

        bool floating = false;
        if (mPos < mText.size() && mText[mPos] == '.') {
            floating = true;
            ++mPos;
            if (mPos >= mText.size() || !std::isdigit(static_cast<unsigned char>(mText[mPos])))
                return false;
            while (mPos < mText.size() && std::isdigit(static_cast<unsigned char>(mText[mPos])))
                ++mPos;
        }

        if (mPos < mText.size() && (mText[mPos] == 'e' || mText[mPos] == 'E')) {
            floating = true;
            ++mPos;
            if (mPos < mText.size() && (mText[mPos] == '+' || mText[mPos] == '-'))
                ++mPos;
            if (mPos >= mText.size() || !std::isdigit(static_cast<unsigned char>(mText[mPos])))
                return false;
            while (mPos < mText.size() && std::isdigit(static_cast<unsigned char>(mText[mPos])))
                ++mPos;
        }

        const std::string literal = mText.substr(start, mPos - start);
        std::istringstream in(literal);
        in.imbue(std::locale::classic());
        if (floating) {
            double value = 0.0;
            in >> value;
            if (!in || !in.eof())
                return false;
            ret = numberValue(mState, value);
        } else {
            int64_t value = 0;
            in >> value;
            if (!in || !in.eof())
                return false;
            ret = naturalValue(mState, value);
        }
        return true;
    }

    NdaState *mState;
    const std::string &mText;
    size_t mPos;
};

bool parseJson(NdaState *state, const std::string &text, NdaVariant &ret)
{
    JsonParser parser(state, text);
    return parser.parse(ret);
}

}

namespace Nda {

void add_AdaJson_symbols(NdaState *state)
{
    assert(state);

    state->registerType("Json", "dict");

    auto stringify = [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        ret.fromString(state->stringType(), serializeJson(args.at("value")));
        return true;
    };

    state->bindFnc("json", "stringify", {{"value", "any", Nda::InMode}}, stringify);
    state->bindFnc("json", "toString", {{"value", "any", Nda::InMode}}, stringify);

    auto parse = [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        return parseJson(state, args.at("text").toString(), ret);
    };

    state->bindFnc("json", "parse", {{"text", "string", Nda::InMode}}, parse);
    state->bindFnc("json", "fromString", {{"text", "string", Nda::InMode}}, parse);

    state->bindFnc("json", "read", {{"file", "textfile", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        std::string text;
        if (!adaIoReadAllTextFile(state, args.at("file"), text))
            return false;
        return parseJson(state, text, ret);
    });

    state->bindPrc("json", "write", {{"file", "textfile", Nda::InMode}, {"value", "any", Nda::InMode}}, [state](const Nda::FncValues& args) -> bool {
        return adaIoWriteTextFile(state, args.at("file"), serializeJson(args.at("value")));
    });
}

}

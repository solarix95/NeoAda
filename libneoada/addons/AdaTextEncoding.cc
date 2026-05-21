#include "AdaTextEncoding.h"
#include "../state.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>

namespace {

enum class EncodingKind {
    Unknown,
    Utf8,
    Utf16,
    Latin1,
    Ascii
};

std::string normalizedEncoding(std::string value)
{
    std::string ret;
    ret.reserve(value.size());
    for (char c : value) {
        if (c == '-' || c == '_' || c == ' ')
            continue;
        ret.push_back((char)std::tolower((unsigned char)c));
    }
    return ret;
}

EncodingKind encodingKind(const std::string &value)
{
    const auto normalized = normalizedEncoding(value);
    if (normalized == "utf8")
        return EncodingKind::Utf8;
    if (normalized == "utf16")
        return EncodingKind::Utf16;
    if (normalized == "latin1" || normalized == "iso88591")
        return EncodingKind::Latin1;
    if (normalized == "ascii" || normalized == "usascii")
        return EncodingKind::Ascii;
    return EncodingKind::Unknown;
}

void appendUtf8(std::string &out, uint32_t codepoint)
{
    if (codepoint <= 0x7f) {
        out.push_back((char)codepoint);
    } else if (codepoint <= 0x7ff) {
        out.push_back((char)(0xc0 | (codepoint >> 6)));
        out.push_back((char)(0x80 | (codepoint & 0x3f)));
    } else if (codepoint <= 0xffff) {
        out.push_back((char)(0xe0 | (codepoint >> 12)));
        out.push_back((char)(0x80 | ((codepoint >> 6) & 0x3f)));
        out.push_back((char)(0x80 | (codepoint & 0x3f)));
    } else {
        out.push_back((char)(0xf0 | (codepoint >> 18)));
        out.push_back((char)(0x80 | ((codepoint >> 12) & 0x3f)));
        out.push_back((char)(0x80 | ((codepoint >> 6) & 0x3f)));
        out.push_back((char)(0x80 | (codepoint & 0x3f)));
    }
}

uint32_t nextUtf8Codepoint(const std::string &text, size_t &pos)
{
    const unsigned char c0 = (unsigned char)text[pos++];
    if (c0 < 0x80)
        return c0;

    auto replacement = [] { return (uint32_t)'?'; };
    if ((c0 & 0xe0) == 0xc0) {
        if (pos >= text.size())
            return replacement();
        const unsigned char c1 = (unsigned char)text[pos++];
        if ((c1 & 0xc0) != 0x80)
            return replacement();
        return ((uint32_t)(c0 & 0x1f) << 6) | (uint32_t)(c1 & 0x3f);
    }

    if ((c0 & 0xf0) == 0xe0) {
        if (pos + 1 >= text.size()) {
            pos = text.size();
            return replacement();
        }
        const unsigned char c1 = (unsigned char)text[pos++];
        const unsigned char c2 = (unsigned char)text[pos++];
        if ((c1 & 0xc0) != 0x80 || (c2 & 0xc0) != 0x80)
            return replacement();
        return ((uint32_t)(c0 & 0x0f) << 12) | ((uint32_t)(c1 & 0x3f) << 6) | (uint32_t)(c2 & 0x3f);
    }

    if ((c0 & 0xf8) == 0xf0) {
        if (pos + 2 >= text.size()) {
            pos = text.size();
            return replacement();
        }
        const unsigned char c1 = (unsigned char)text[pos++];
        const unsigned char c2 = (unsigned char)text[pos++];
        const unsigned char c3 = (unsigned char)text[pos++];
        if ((c1 & 0xc0) != 0x80 || (c2 & 0xc0) != 0x80 || (c3 & 0xc0) != 0x80)
            return replacement();
        return ((uint32_t)(c0 & 0x07) << 18) | ((uint32_t)(c1 & 0x3f) << 12) | ((uint32_t)(c2 & 0x3f) << 6) | (uint32_t)(c3 & 0x3f);
    }

    return replacement();
}

NdaVariant byteValue(NdaState *state, unsigned char value)
{
    NdaVariant ret;
    ret.fromByte(state->typeByName("byte"), value);
    return ret;
}

bool appendByte(NdaState *state, NdaVariant &bytes, unsigned char value)
{
    return bytes.appendToBytes(byteValue(state, value));
}

void appendUtf16Le(NdaState *state, NdaVariant &bytes, uint16_t value)
{
    appendByte(state, bytes, (unsigned char)(value & 0xff));
    appendByte(state, bytes, (unsigned char)((value >> 8) & 0xff));
}

bool encodeUtf16(NdaState *state, const std::string &text, NdaVariant &ret)
{
    appendByte(state, ret, 0xff);
    appendByte(state, ret, 0xfe);

    size_t pos = 0;
    while (pos < text.size()) {
        uint32_t cp = nextUtf8Codepoint(text, pos);
        if (cp > 0x10ffff)
            cp = '?';

        if (cp <= 0xffff) {
            appendUtf16Le(state, ret, (uint16_t)cp);
        } else {
            cp -= 0x10000;
            appendUtf16Le(state, ret, (uint16_t)(0xd800 | (cp >> 10)));
            appendUtf16Le(state, ret, (uint16_t)(0xdc00 | (cp & 0x3ff)));
        }
    }

    return true;
}

bool readByte(const NdaVariant &bytes, int index, unsigned char &ret)
{
    bool ok;
    auto value = bytes.readBytesAccess(index).toInt64(&ok);
    if (!ok)
        return false;
    ret = (unsigned char)value;
    return true;
}

uint16_t readUtf16Unit(unsigned char first, unsigned char second, bool littleEndian)
{
    if (littleEndian)
        return (uint16_t)first | ((uint16_t)second << 8);
    return ((uint16_t)first << 8) | (uint16_t)second;
}

bool decodeUtf16(const NdaVariant &bytes, std::string &ret)
{
    int pos = 0;
    bool littleEndian = true;
    const int len = bytes.lengthOperator();

    if (len >= 2) {
        unsigned char b0;
        unsigned char b1;
        if (!readByte(bytes, 0, b0) || !readByte(bytes, 1, b1))
            return false;
        if (b0 == 0xff && b1 == 0xfe) {
            littleEndian = true;
            pos = 2;
        } else if (b0 == 0xfe && b1 == 0xff) {
            littleEndian = false;
            pos = 2;
        }
    }

    while (pos + 1 < len) {
        unsigned char b0;
        unsigned char b1;
        if (!readByte(bytes, pos, b0) || !readByte(bytes, pos + 1, b1))
            return false;
        pos += 2;

        uint32_t cp = readUtf16Unit(b0, b1, littleEndian);
        if (cp >= 0xd800 && cp <= 0xdbff) {
            if (pos + 1 >= len) {
                appendUtf8(ret, '?');
                break;
            }
            unsigned char b2;
            unsigned char b3;
            if (!readByte(bytes, pos, b2) || !readByte(bytes, pos + 1, b3))
                return false;
            pos += 2;

            uint32_t low = readUtf16Unit(b2, b3, littleEndian);
            if (low >= 0xdc00 && low <= 0xdfff) {
                cp = 0x10000 + (((cp - 0xd800) << 10) | (low - 0xdc00));
            } else {
                appendUtf8(ret, '?');
                continue;
            }
        } else if (cp >= 0xdc00 && cp <= 0xdfff) {
            cp = '?';
        }

        appendUtf8(ret, cp);
    }

    return true;
}

bool encodeText(NdaState *state, const std::string &text, EncodingKind encoding, NdaVariant &ret)
{
    ret.initType(state->bytesType());

    switch (encoding) {
    case EncodingKind::Utf8:
        for (unsigned char c : text)
            appendByte(state, ret, c);
        return true;
    case EncodingKind::Utf16:
        return encodeUtf16(state, text, ret);
    case EncodingKind::Latin1: {
        size_t pos = 0;
        while (pos < text.size()) {
            uint32_t cp = nextUtf8Codepoint(text, pos);
            appendByte(state, ret, cp <= 0xff ? (unsigned char)cp : (unsigned char)'?');
        }
        return true;
    }
    case EncodingKind::Ascii: {
        size_t pos = 0;
        while (pos < text.size()) {
            uint32_t cp = nextUtf8Codepoint(text, pos);
            appendByte(state, ret, cp <= 0x7f ? (unsigned char)cp : (unsigned char)'?');
        }
        return true;
    }
    case EncodingKind::Unknown:
        return false;
    }
    return false;
}

bool decodeText(const NdaVariant &bytes, EncodingKind encoding, std::string &ret)
{
    ret.clear();

    switch (encoding) {
    case EncodingKind::Utf8:
        for (int i = 0; i < bytes.lengthOperator(); ++i) {
            bool ok;
            auto value = bytes.readBytesAccess(i).toInt64(&ok);
            if (!ok)
                return false;
            ret.push_back((char)(unsigned char)value);
        }
        return true;
    case EncodingKind::Utf16:
        return decodeUtf16(bytes, ret);
    case EncodingKind::Latin1:
        for (int i = 0; i < bytes.lengthOperator(); ++i) {
            bool ok;
            auto value = bytes.readBytesAccess(i).toInt64(&ok);
            if (!ok)
                return false;
            appendUtf8(ret, (uint32_t)(unsigned char)value);
        }
        return true;
    case EncodingKind::Ascii:
        for (int i = 0; i < bytes.lengthOperator(); ++i) {
            bool ok;
            auto value = bytes.readBytesAccess(i).toInt64(&ok);
            if (!ok)
                return false;
            const unsigned char c = (unsigned char)value;
            ret.push_back(c <= 0x7f ? (char)c : '?');
        }
        return true;
    case EncodingKind::Unknown:
        return false;
    }
    return false;
}

}

namespace Nda {

bool encodeTextBytes(NdaState *state, const std::string &text, const std::string &encoding, NdaVariant &ret)
{
    auto kind = encodingKind(encoding);
    if (kind == EncodingKind::Unknown) {
        ret.reset();
        return false;
    }
    return encodeText(state, text, kind, ret);
}

bool decodeTextBytes(const NdaVariant &bytes, const std::string &encoding, std::string &ret)
{
    auto kind = encodingKind(encoding);
    if (kind == EncodingKind::Unknown) {
        ret.clear();
        return false;
    }
    return decodeText(bytes, kind, ret);
}

void add_AdaTextEncoding_symbols(NdaState *state)
{
    state->registerType("Encoding", "dict");

    state->bindFnc("encoding", "encode", {{"text", "string", Nda::InMode}, {"encoding", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        return encodeTextBytes(state, args.at("text").toString(), args.at("encoding").toString(), ret);
    });

    state->bindFnc("encoding", "decode", {{"data", "bytes", Nda::InMode}, {"encoding", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        std::string text;
        if (!decodeTextBytes(args.at("data"), args.at("encoding").toString(), text))
            return false;

        ret.fromString(state->stringType(), text);
        return true;
    });
}

}

#include "AdaString.h"
#include "AdaTextEncoding.h"
#include "../state.h"
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <iostream>

#define CHECK_INSTANCE_CALL if (args.find("this") == args.end()) return false

namespace Nda {

inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());
}

std::string trimmed(std::string s)
{
    rtrim(s);
    ltrim(s);
    return s;
}

std::string lowerAscii(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return s;
}

bool parseNumber(const std::string &text, double &value)
{
    const std::string s = trimmed(text);
    if (s.empty())
        return false;

    char *end = nullptr;
    errno = 0;
    value = std::strtod(s.c_str(), &end);
    return true; // errno == 0 && end && *end == '\0';
}

bool parseNatural(const std::string &text, int64_t &value)
{
    const std::string s = trimmed(text);
    if (s.empty() || s[0] == '-')
        return false;

    char *end = nullptr;
    errno = 0;
    const long long parsed = std::strtoll(s.c_str(), &end, 10);
    if (errno != 0 || !end || *end != '\0' || parsed < 0)
        return false;

    value = static_cast<int64_t>(parsed);
    return true;
}

bool parseBool(const std::string &text, bool &value)
{
    const std::string s = lowerAscii(trimmed(text));
    if (s == "true") {
        value = true;
        return true;
    }
    if (s == "false") {
        value = false;
        return true;
    }
    return false;
}

void add_AdaString_symbols(NdaState *state)
{
    assert(state);

    // ------------------ String.Length() ---------------------------------------------------------
    state->bindFnc("string","length",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() == Nda::String)
            ret.fromNatural(state->naturalType(),self.lengthOperator());
        else
            return false;
        return true;
    });

    // ------------------ String.Append() ---------------------------------------------------------
    state->bindPrc("string","append",{{"s", "any", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");
        auto element = args.at("s");

        if (self.type() != Nda::String)
            return false;

        bool done;
        self.assign(self.concat(element,&done));
        return done;
    });

    // ------------------ String.ToUpper() ---------------------------------------------------------
    state->bindFnc("string","toUpper",{}, [](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        std::string s = self.toString();
        std::transform(s.begin(), s.end(), s.begin(), ::toupper); // https://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case

        ret.fromString(self.runtimeType(),s);
        return true;
    });

    // ------------------ String.ToLower() ---------------------------------------------------------
    state->bindFnc("string","toLower",{}, [](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        std::string s = self.toString();
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);

        ret.fromString(self.runtimeType(),s);
        return true;
    });

    // ------------------ String.Upper() ---------------------------------------------------------
    state->bindPrc("string","upper",{}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        std::string s = self.toString();
        std::transform(s.begin(), s.end(), s.begin(), ::toupper); // https://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case

        self.setString(s);
        return true;
    });

    // ------------------ String.Upper() ---------------------------------------------------------
    state->bindPrc("string","lower",{}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        std::string s = self.toString();
        std::transform(s.begin(), s.end(), s.begin(), ::tolower); // https://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case

        self.setString(s);
        return true;
    });

    // ------------------ String.Contains() ---------------------------------------------------------
    state->bindFnc("string","contains",{{"s", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self   = args.at("this");
        auto needle = args.at("s");

        if (self.type() != Nda::String)
            return false;

        ret.fromBool(state->booleanType(),self.toString().find(needle.toString()) != std::string::npos);
        return true;
    });

    // ------------------ String.IndexOf() ---------------------------------------------------------
    state->bindFnc("string","indexOf",{{"s", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self   = args.at("this");
        auto needle = args.at("s");

        if (self.type() != Nda::String)
            return false;

        std::string::size_type loc = self.toString().find(needle.toString(), 0 );

        ret.fromNatural(state->naturalType(),(loc != std::string::npos) ? loc : -1);
        return true;
    });

    // ------------------ String.Upper() ---------------------------------------------------------
    state->bindPrc("string","insert",{{"i", "natural", Nda::InMode},{"s", "string", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self  = args.at("this");
        auto index = args.at("i").toInt64();
        auto str   = args.at("s").toString();

        if (self.type() != Nda::String)
            return false;

        self.setString(self.toString().insert(index,str));
        return true;
    });

    // ------------------ String.Trim() ---------------------------------------------------------
    state->bindPrc("string","trim",{}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        std::string s = self.toString();
        rtrim(s);
        ltrim(s);
        self.setString(s);
        return true;
    });

    // ------------------ String.Trimmed() ---------------------------------------------------------
    state->bindFnc("string","trimmed",{}, [](const Nda::FncValues& args,  NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        std::string s = self.toString();
        rtrim(s);
        ltrim(s);

        ret.fromString(self.runtimeType(),s);
        return true;
    });

    // ------------------ String.Chop() ---------------------------------------------------------
    state->bindPrc("string","chop",{{"i", "natural", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self  = args.at("this");
        auto count = args.at("i").toInt64();

        if (self.type() != Nda::String)
            return false;

        std::string s = self.toString();
        s = count >= s.length() ? "" : s.substr(0,s.length()-count);
        self.setString(s);

        return true;
    });

    // ------------------ String.Chopped() ---------------------------------------------------------
    state->bindFnc("string","chopped",{{"i", "natural", Nda::InMode}}, [](const Nda::FncValues& args,  NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self  = args.at("this");
        auto count = args.at("i").toInt64();

        if (self.type() != Nda::String)
            return false;

        std::string s = self.toString();
        s = count >= s.length() ? "" : s.substr(0,s.length()-count);
        ret.fromString(self.runtimeType(),s);

        return true;
    });

    // ------------------ String.Slice() ---------------------------------------------------------
    state->bindPrc("string","slice",{{"pos", "natural", Nda::InMode},{"n", "natural", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self  = args.at("this");
        auto pos   = args.at("pos").toInt64();
        auto count = args.at("n").toInt64();

        if (self.type() != Nda::String)
            return false;

        std::string s = self.toString();
        s = pos >= s.length() ? "" : s.substr(pos,count);
        self.setString(s);

        return true;
    });

    // ------------------ String.Sliced() ---------------------------------------------------------
    state->bindFnc("string","sliced",{{"pos", "natural", Nda::InMode},{"n", "natural", Nda::InMode}}, [](const Nda::FncValues& args,  NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self  = args.at("this");
        auto pos   = args.at("pos").toInt64();
        auto count = args.at("n").toInt64();

        if (self.type() != Nda::String)
            return false;

        std::string s = self.toString();
        s = pos >= s.length() ? "" : s.substr(pos,count);
        ret.fromString(self.runtimeType(),s);

        return true;
    });

    // ------------------ String.FromBytes() ---------------------------------------------------------
    state->bindFnc("string","fromBytes",{{"data", "bytes", Nda::InMode},{"encoding", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        std::string text;
        if (!decodeTextBytes(args.at("data"), args.at("encoding").toString(), text))
            return false;

        ret.fromString(state->stringType(), text);
        return true;
    });


    // ------------------ String.ToNumber() ---------------------------------------------------------
    state->bindFnc("string","toNumber",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        std::cout << "TO NUMBER1 " << self.runtimeType()->name.lowerValue << std::endl;
        if (self.type() != Nda::String)
            return false;

        double value = 0.0;

        std::cout << "TO NUMBER2 " << self.toString() << " " << parseNumber(self.toString(), value) << std::endl;
        if (!parseNumber(self.toString(), value))
            return false;

        ret.fromNumber(state->numberType(), value);
        return true;
    });

    // ------------------ String.ToNatural() ---------------------------------------------------------
    state->bindFnc("string","toNatural",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        int64_t value = 0;
        if (!parseNatural(self.toString(), value))
            return false;

        ret.fromNatural(state->naturalType(), value);
        return true;
    });

    // ------------------ String.ToBool() ---------------------------------------------------------
    state->bindFnc("string","toBool",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        bool value = false;
        if (!parseBool(self.toString(), value))
            return false;

        ret.fromBool(state->booleanType(), value);
        return true;
    });

    // ------------------ String.IsNumber() ---------------------------------------------------------
    state->bindFnc("string","isNumber",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        double value = 0.0;
        ret.fromBool(state->booleanType(), parseNumber(self.toString(), value));
        return true;
    });

    // ------------------ String.IsNatural() ---------------------------------------------------------
    state->bindFnc("string","isNatural",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        int64_t value = 0;
        ret.fromBool(state->booleanType(), parseNatural(self.toString(), value));
        return true;
    });

    // ------------------ String.IsBool() ---------------------------------------------------------
    state->bindFnc("string","isBool",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        bool value = false;
        ret.fromBool(state->booleanType(), parseBool(self.toString(), value));
        return true;
    });

    // ------------------ String.ToBytes() ---------------------------------------------------------
    state->bindFnc("string","toBytes",{{"encoding", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::String)
            return false;

        return encodeTextBytes(state, self.toString(), args.at("encoding").toString(), ret);
    });
}

}

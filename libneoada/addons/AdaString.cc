#include "AdaString.h"
#include "../state.h"
#include <algorithm>
#include <cassert>

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

    // ------------------ String.Trim() ---------------------------------------------------------
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

    // ------------------ String.Upper() ---------------------------------------------------------
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
}

}

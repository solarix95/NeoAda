#include "AdaRegexp.h"
#include "../state.h"

#include <cassert>
#include <regex>
#include <string>

namespace {

NdaVariant stringValue(NdaState *state, const std::string &value)
{
    NdaVariant ret;
    ret.fromString(state->stringType(), value);
    return ret;
}

void retBool(NdaState *state, NdaVariant &ret, bool value)
{
    ret.fromBool(state->booleanType(), value);
}

NdaVariant listValue(NdaState *state)
{
    NdaVariant ret;
    ret.initType(state->listType());
    return ret;
}

std::regex makeRegex(const std::string &pattern)
{
    return std::regex(pattern, std::regex::ECMAScript);
}

}

namespace Nda {

void add_AdaRegexp_symbols(NdaState *state)
{
    assert(state);

    state->registerType("Regexp", "dict");

    state->bindFnc("regexp", "match", {{"text", "string", Nda::InMode}, {"pattern", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        try {
            retBool(state, ret, std::regex_match(args.at("text").toString(), makeRegex(args.at("pattern").toString())));
            return true;
        } catch (const std::regex_error&) {
            return false;
        }
    });

    state->bindFnc("regexp", "contains", {{"text", "string", Nda::InMode}, {"pattern", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        try {
            retBool(state, ret, std::regex_search(args.at("text").toString(), makeRegex(args.at("pattern").toString())));
            return true;
        } catch (const std::regex_error&) {
            return false;
        }
    });

    state->bindFnc("regexp", "firstMatch", {{"text", "string", Nda::InMode}, {"pattern", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        try {
            const std::string text = args.at("text").toString();
            std::smatch match;
            if (std::regex_search(text, match, makeRegex(args.at("pattern").toString())))
                ret.fromString(state->stringType(), match.str(0));
            else
                ret.fromString(state->stringType(), std::string());
            return true;
        } catch (const std::regex_error&) {
            return false;
        }
    });

    state->bindFnc("regexp", "captures", {{"text", "string", Nda::InMode}, {"pattern", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        try {
            ret = listValue(state);
            const std::string text = args.at("text").toString();
            std::smatch match;
            if (std::regex_search(text, match, makeRegex(args.at("pattern").toString()))) {
                for (size_t i = 0; i < match.size(); ++i)
                    ret.appendToList(stringValue(state, match.str(i)));
            }
            return true;
        } catch (const std::regex_error&) {
            return false;
        }
    });

    state->bindFnc("regexp", "replace", {{"text", "string", Nda::InMode}, {"pattern", "string", Nda::InMode}, {"replacement", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        try {
            ret.fromString(state->stringType(), std::regex_replace(args.at("text").toString(), makeRegex(args.at("pattern").toString()), args.at("replacement").toString()));
            return true;
        } catch (const std::regex_error&) {
            return false;
        }
    });

    state->bindFnc("regexp", "split", {{"text", "string", Nda::InMode}, {"pattern", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        try {
            ret = listValue(state);
            const std::string text = args.at("text").toString();
            const std::regex regex = makeRegex(args.at("pattern").toString());
            std::sregex_token_iterator it(text.begin(), text.end(), regex, -1);
            std::sregex_token_iterator end;
            for (; it != end; ++it)
                ret.appendToList(stringValue(state, it->str()));
            return true;
        } catch (const std::regex_error&) {
            return false;
        }
    });
}

}

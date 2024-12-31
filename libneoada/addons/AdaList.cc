#include "AdaList.h"
#include "../state.h"
#include <cassert>

#define CHECK_INSTANCE_CALL if (args.find("this") == args.end()) return false

namespace Nda {

void add_AdaList_symbols(NdaState *state)
{
    assert(state);

    // {{"n", "natural", Nda::InMode}}

    // ------------------ String.Length() ---------------------------------------------------------
    state->bindFnc("list","length",{}, [](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() == Nda::List)
            ret.fromNumber((int64_t)self.listSize());
        else
            return false;
        return true;
    });

    // ------------------ String.Append() ---------------------------------------------------------
    state->bindPrc("list","append",{{"v", "any", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return false;

        self.appendToList(element);
        return true;
    });

    // ------------------ String.Insert() ---------------------------------------------------------
    state->bindPrc("list","insert",{{"p", "Number", Nda::InMode}, {"v", "any", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");
        auto pos     = args.at("p");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return false;

        self.insertIntoList((int)pos.toInt64(), element);
        return true;
    });

    // ------------------ String.Concat() ---------------------------------------------------------
    state->bindPrc("list","concat",{{"v", "any", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return false;

        assert(self.myType() == Nda::Reference);

        bool done;
        auto ret = self.concat(element, &done);
        if (!done)
            return false;

        self.assign(ret);
        return true;
    });

    // ------------------ String.Append() ---------------------------------------------------------
    state->bindFnc("list","contains",{{"v", "any", Nda::InMode}}, [](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return false;

        ret.fromBool(self.containsInList(element));
        return true;
    });

    // ------------------ String.Append() ---------------------------------------------------------
    state->bindFnc("list","indexOf",{{"v", "any", Nda::InMode}}, [](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return false;

        ret.fromNumber((int64_t)self.indexInList(element));
        return true;
    });



}

}

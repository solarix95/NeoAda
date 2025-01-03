#include "AdaList.h"
#include "../state.h"
#include <cassert>

#define CHECK_INSTANCE_CALL if (args.find("this") == args.end()) return false

namespace Nda {

void add_AdaList_symbols(NdaState *state)
{
    assert(state);

    // ------------------ List.Length() ---------------------------------------------------------
    state->bindFnc("list","length",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() == Nda::List)
            ret.fromNatural(state->typeByName("natural"),self.listSize());
        else
            return false;
        return true;
    });

    // ------------------ List.Clear() ---------------------------------------------------------
    state->bindPrc("list","clear",{}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;

        self.clearList();
        return true;
    });

    // ------------------ List.Append() ---------------------------------------------------------
    state->bindPrc("list","append",{{"v", "any", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return false;

        self.appendToList(element);
        return true;
    });

    // ------------------ List.Insert() ---------------------------------------------------------
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

    // ------------------ List.Concat() ---------------------------------------------------------
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

    // ------------------ List.Contains() ---------------------------------------------------------
    state->bindFnc("list","contains",{{"v", "any", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return false;

        ret.fromBool(state->typeByName("boolean"),self.containsInList(element));
        return true;
    });

    // ------------------ List.IndexOf() ---------------------------------------------------------
    state->bindFnc("list","indexOf",{{"v", "any", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return false;

        ret.fromNatural(state->typeByName("natural"),(int64_t)self.indexInList(element));
        return true;
    });

    // ------------------ List.Flip() ---------------------------------------------------------
    state->bindPrc("list","flip",{}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");

        if (self.type() != Nda::List)
            return false;

        self.reverseList();

        return true;
    });

    // ------------------ List.Flipped() ---------------------------------------------------------
    state->bindFnc("list","flipped",{}, [](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self    = args.at("this");

        if (self.type() != Nda::List)
            return false;

        ret.initType(self.runtimeType());
        ret.assign(self);
        ret.reverseList();

        return true;
    });




}

}

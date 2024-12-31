#include "AdaList.h"
#include "../state.h"
#include <cassert>

namespace Nda {

void add_AdaList_symbols(NdaState *state)
{
    assert(state);

    // {{"n", "natural", Nda::InMode}}

    // ------------------ String.Length() ---------------------------------------------------------
    state->bindFnc("list","length",{}, [](const Nda::FncValues& args) -> NdaVariant {

        auto self = args.at("this");

        NdaVariant ret;
        if (self.type() == Nda::List)
            ret.fromNumber((int64_t)self.listSize());

        return ret;
    });

    // ------------------ String.Append() ---------------------------------------------------------
    state->bindPrc("list","append",{{"v", "any", Nda::InMode}}, [](const Nda::FncValues& args) -> void {

        auto self    = args.at("this");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return;

        self.appendToList(element);
    });

    // ------------------ String.Insert() ---------------------------------------------------------
    state->bindPrc("list","insert",{{"p", "Number", Nda::InMode}, {"v", "any", Nda::InMode}}, [](const Nda::FncValues& args) -> void {

        auto self    = args.at("this");
        auto pos     = args.at("p");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return;

        self.insertIntoList((int)pos.toInt64(), element);
    });

    // ------------------ String.Concat() ---------------------------------------------------------
    state->bindPrc("list","concat",{{"v", "any", Nda::InMode}}, [](const Nda::FncValues& args) -> void {

        auto self    = args.at("this");
        auto element = args.at("v");

        if (self.type() != Nda::List)
            return;

        assert(self.myType() == Nda::Reference);

        bool done;
        auto ret = self.concat(element, &done);
        if (done)
            self.assign(ret);
    });



}

}

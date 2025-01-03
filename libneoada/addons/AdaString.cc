#include "AdaString.h"
#include "../state.h"
#include <cassert>

#define CHECK_INSTANCE_CALL if (args.find("this") == args.end()) return false

namespace Nda {

void add_AdaString_symbols(NdaState *state)
{
    assert(state);

    // ------------------ List.Length() ---------------------------------------------------------
    state->bindFnc("string","length",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() == Nda::String)
            ret.fromNatural(state->typeByName("natural"),self.lengthOperator());
        else
            return false;
        return true;
    });


}

}

#include "AdaList.h"
#include "../state.h"
#include <cassert>
#include <algorithm>

#define CHECK_INSTANCE_CALL if (args.find("this") == args.end()) return false

namespace Nda {

void add_AdaList_symbols(NdaState *state)
{
    assert(state);

    // ------------------ List:Version() ---------------------------------------------------------
    /*
    state->bind("list","version",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() == Nda::List)
            ret.fromNatural(state->typeByName("natural"),self.lengthOperator());
        else
            return false;
        return true;
    });
    */

    // ------------------ List.Length() ---------------------------------------------------------
    state->bindFnc("list","length",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() == Nda::List)
            ret.fromNatural(state->typeByName("natural"),self.lengthOperator());
        else
            return false;
        return true;
    });

    // ------------------ List.IsEmpty() -------------------------------------------------------
    state->bindFnc("list","isEmpty",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {

        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;

        ret.fromBool(state->booleanType(), self.listSize() == 0);
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

    // ------------------ List.Extend() ---------------------------------------------------------
    state->bindPrc("list","extend",{{"values", "list", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {

        CHECK_INSTANCE_CALL;

        auto self   = args.at("this");
        auto values = args.at("values");

        if (self.type() != Nda::List || values.type() != Nda::List)
            return false;

        const int count = values.listSize();
        for (int i=0; i<count; i++)
            self.appendToList(values.readAccess(i));
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

    // ------------------ List.First() ---------------------------------------------------------
    state->bindFnc("list","first",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;
        if (self.listSize() <= 0) {
            state->raiseException("constrainterror");
            return false;
        }

        ret = self.readAccess(0);
        return true;
    });

    // ------------------ List.Last() ----------------------------------------------------------
    state->bindFnc("list","last",{}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;
        if (self.listSize() <= 0) {
            state->raiseException("constrainterror");
            return false;
        }

        ret = self.readAccess(self.listSize() - 1);
        return true;
    });

    // ------------------ List.TakeAt() --------------------------------------------------------
    state->bindFnc("list", "takeAt", {{"pos", "natural", Nda::InMode}}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;

        bool ok = false;
        const int64_t pos = args.at("pos").toInt64(&ok);
        if (!ok || pos < 0 || pos >= self.listSize()) {
            state->raiseException("constrainterror");
            return false;
        }

        ret = self.readAccess(static_cast<int>(pos));
        self.takeFromList(static_cast<int>(pos));
        return true;
    });

    // ------------------ List.TakeFirst() -----------------------------------------------------
    state->bindFnc("list", "takeFirst", {}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;
        if (self.listSize() <= 0) {
            state->raiseException("constrainterror");
            return false;
        }

        ret = self.readAccess(0);
        self.takeFromList(0);
        return true;
    });

    // ------------------ List.TakeLast() ------------------------------------------------------
    state->bindFnc("list", "takeLast", {}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;
        if (self.listSize() <= 0) {
            state->raiseException("constrainterror");
            return false;
        }

        ret = self.readAccess(self.listSize() - 1);
        self.takeFromList(self.listSize() - 1);
        return true;
    });

    // ------------------ List.RemoveAt() -------------------------------------------------------
    state->bindPrc("list", "removeAt", {{"pos", "natural", Nda::InMode}}, [state](const Nda::FncValues &args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;

        bool ok = false;
        const int64_t pos = args.at("pos").toInt64(&ok);
        if (!ok || pos < 0 || pos >= self.listSize()) {
            state->raiseException("constrainterror");
            return false;
        }

        self.takeFromList(static_cast<int>(pos));
        return true;
    });

    // ------------------ List.RemoveFirst() ----------------------------------------------------
    state->bindPrc("list", "removeFirst", {}, [state](const Nda::FncValues &args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;
        if (self.listSize() <= 0) {
            state->raiseException("constrainterror");
            return false;
        }

        self.takeFromList(0);
        return true;
    });

    // ------------------ List.RemoveLast() -----------------------------------------------------
    state->bindPrc("list", "removeLast", {}, [state](const Nda::FncValues &args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;
        if (self.listSize() <= 0) {
            state->raiseException("constrainterror");
            return false;
        }

        self.takeFromList(self.listSize() - 1);
        return true;
    });

    // ------------------ List.Mid() -----------------------------------------------------------
    state->bindFnc("list", "mid", {{"pos", "natural", Nda::InMode}, {"n", "natural", Nda::InMode}}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;

        bool posOk = false;
        bool countOk = false;
        const int64_t pos = args.at("pos").toInt64(&posOk);
        const int64_t count = args.at("n").toInt64(&countOk);
        if (!posOk || !countOk || pos < 0 || count < 0) {
            state->raiseException("constrainterror");
            return false;
        }

        ret.initType(state->listType());
        const int size = self.listSize();
        if (pos >= size)
            return true;

        const int end = std::min<int64_t>(size, pos + count);
        for (int i=static_cast<int>(pos); i<end; i++)
            ret.appendToList(self.readAccess(i));
        return true;
    });

    // ------------------ List.Sort() ----------------------------------------------------------
    state->bindPrc("list", "sort", {}, [](const Nda::FncValues &args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;

        self.sortList();
        return true;
    });

    // ------------------ List.Sorted() --------------------------------------------------------
    state->bindFnc("list", "sorted", {}, [](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::List)
            return false;

        ret.initType(self.runtimeType());
        ret.assign(self);
        ret.sortList();
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

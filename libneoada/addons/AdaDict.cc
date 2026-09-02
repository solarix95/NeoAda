#include "AdaDict.h"
#include "../state.h"

#include <cassert>

#define CHECK_INSTANCE_CALL if (args.find("this") == args.end()) return false

namespace Nda {

void add_AdaDict_symbols(NdaState *state)
{
    assert(state);

    state->bindFnc("dict", "length", {}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        const auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        ret.fromNatural(state->naturalType(), self.dictSize());
        return true;
    });


    state->bindFnc("dict", "isEmpty", {}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        const auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        ret.fromBool(state->booleanType(), self.dictSize() == 0);
        return true;
    });

    state->bindPrc("dict", "clear", {}, [](const Nda::FncValues &args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        const auto items = self.dictItems();
        for (const auto &item : items)
            self.takeFromDict(item.first);
        return true;
    });

    state->bindFnc("dict", "contains", {{"key", "any", Nda::InMode}}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        const auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        ret.fromBool(state->booleanType(), self.contains(args.at("key")));
        return true;
    });

    state->bindFnc("dict", "remove", {{"key", "any", Nda::InMode}}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        const bool removed = self.contains(args.at("key"));
        if (removed)
            self.takeFromDict(args.at("key"));
        ret.fromNatural(state->naturalType(), removed ? 1 : 0);
        return true;
    });

    state->bindFnc("dict", "keys", {}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        const auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        ret.initType(state->listType());
        for (const auto &item : self.dictItems())
            ret.appendToList(item.first);
        return true;
    });

    state->bindFnc("dict", "values", {}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        const auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        ret.initType(state->listType());
        for (const auto &item : self.dictItems())
            ret.appendToList(item.second);
        return true;
    });

    state->bindFnc("dict", "value", {{"key", "any", Nda::InMode}, {"defaultValue", "any", Nda::InMode}}, [](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        const auto key = args.at("key");
        ret = self.contains(key) ? self.writeDictAccess(key) : args.at("defaultValue");
        return true;
    });

    state->bindFnc("dict", "items", {}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        const auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        NdaVariant keyName;
        keyName.fromString(state->stringType(), "key");
        NdaVariant valueName;
        valueName.fromString(state->stringType(), "value");

        ret.initType(state->listType());
        for (const auto &item : self.dictItems()) {
            NdaVariant entry;
            entry.initType(state->dictType());
            entry.appendToDict(keyName, item.first);
            entry.appendToDict(valueName, item.second);
            ret.appendToList(entry);
        }
        return true;
    });

    state->bindFnc("dict", "take", {{"key", "any", Nda::InMode}}, [state](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        const auto key = args.at("key");
        if (!self.contains(key)) {
            state->raiseException("constrainterror");
            return false;
        }

        ret = self.writeDictAccess(key);
        self.takeFromDict(key);
        return true;
    });

    state->bindFnc("dict", "take", {{"key", "any", Nda::InMode}, {"defaultValue", "any", Nda::InMode}}, [](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        const auto key = args.at("key");
        if (self.contains(key)) {
            ret = self.writeDictAccess(key);
            self.takeFromDict(key);
        } else {
            ret = args.at("defaultValue");
        }
        return true;
    });

    state->bindFnc("dict", "ensure", {{"key", "any", Nda::InMode}, {"defaultValue", "any", Nda::InMode}}, [](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Dict)
            return false;

        const auto key = args.at("key");
        if (!self.contains(key))
            self.appendToDict(key, args.at("defaultValue"));
        ret = self.writeDictAccess(key);
        return true;
    });

    state->bindPrc("dict", "update", {{"other", "dict", Nda::InMode}}, [](const Nda::FncValues &args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        const auto other = args.at("other");
        if (self.type() != Nda::Dict || other.type() != Nda::Dict)
            return false;

        for (const auto &item : other.dictItems())
            self.appendToDict(item.first, item.second);
        return true;
    });

    state->bindFnc("dict", "updated", {{"other", "dict", Nda::InMode}}, [](const Nda::FncValues &args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        const auto other = args.at("other");
        if (self.type() != Nda::Dict || other.type() != Nda::Dict)
            return false;

        ret = self;
        ret.dereference();
        for (const auto &item : other.dictItems())
            ret.appendToDict(item.first, item.second);
        return true;
    });

}

}

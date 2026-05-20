#include "AdaBytes.h"
#include "../state.h"
#include <cassert>

#define CHECK_INSTANCE_CALL if (args.find("this") == args.end()) return false

namespace Nda {

static NdaVariant bytesRange(const NdaVariant &source, const Nda::RuntimeType *bytesType, int64_t pos, int64_t count)
{
    NdaVariant ret;
    ret.initType(bytesType);

    int64_t size = source.lengthOperator();
    if (pos < 0)
        pos = 0;
    if (count < 0)
        count = 0;
    if (pos >= size)
        return ret;

    int64_t end = pos + count;
    if (end > size)
        end = size;

    for (int64_t i = pos; i < end; ++i)
        ret.appendToBytes(source.readBytesAccess((int)i));

    return ret;
}

static int64_t bytesIndexOf(const NdaVariant &source, const NdaVariant &needle)
{
    for (int i = 0; i < source.lengthOperator(); ++i) {
        if (source.readBytesAccess(i).equal(needle))
            return i;
    }

    return -1;
}

void add_AdaBytes_symbols(NdaState *state)
{
    assert(state);

    // ------------------ Bytes.Length() ---------------------------------------------------------
    state->bindFnc("bytes", "length", {}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Bytes)
            return false;

        ret.fromNatural(state->naturalType(), self.lengthOperator());
        return true;
    });

    // ------------------ Bytes.Append(Byte) -----------------------------------------------------
    state->bindPrc("bytes", "append", {{"v", "byte", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Bytes)
            return false;

        return self.appendToBytes(args.at("v"));
    });

    // ------------------ Bytes.Append(Bytes) ----------------------------------------------------
    state->bindPrc("bytes", "append", {{"v", "bytes", Nda::InMode}}, [](const Nda::FncValues& args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        auto other = args.at("v");
        if (self.type() != Nda::Bytes || other.type() != Nda::Bytes)
            return false;

        for (int i = 0; i < other.lengthOperator(); ++i) {
            if (!self.appendToBytes(other.readBytesAccess(i)))
                return false;
        }

        return true;
    });

    // ------------------ Bytes.Clear() ----------------------------------------------------------
    state->bindPrc("bytes", "clear", {}, [](const Nda::FncValues& args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Bytes)
            return false;

        self.clearBytes();
        return true;
    });

    // ------------------ Bytes.Contains() -------------------------------------------------------
    state->bindFnc("bytes", "contains", {{"v", "byte", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Bytes)
            return false;

        ret.fromBool(state->booleanType(), bytesIndexOf(self, args.at("v")) >= 0);
        return true;
    });

    // ------------------ Bytes.IndexOf() --------------------------------------------------------
    state->bindFnc("bytes", "indexOf", {{"v", "byte", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Bytes)
            return false;

        ret.fromNatural(state->naturalType(), bytesIndexOf(self, args.at("v")));
        return true;
    });

    // ------------------ Bytes.Insert() ---------------------------------------------------------
    state->bindPrc("bytes", "insert", {{"pos", "natural", Nda::InMode}, {"v", "byte", Nda::InMode}}, [state](const Nda::FncValues& args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        auto pos = args.at("pos").toInt64();
        auto value = args.at("v");
        if (self.type() != Nda::Bytes || value.type() != Nda::Byte)
            return false;

        int64_t size = self.lengthOperator();
        if (pos < 0)
            pos = 0;
        if (pos > size)
            pos = size;

        NdaVariant next;
        next.initType(state->bytesType());
        for (int64_t i = 0; i < pos; ++i)
            next.appendToBytes(self.readBytesAccess((int)i));
        next.appendToBytes(value);
        for (int64_t i = pos; i < size; ++i)
            next.appendToBytes(self.readBytesAccess((int)i));

        return self.assign(next);
    });

    // ------------------ Bytes.Remove() ---------------------------------------------------------
    state->bindPrc("bytes", "remove", {{"pos", "natural", Nda::InMode}, {"n", "natural", Nda::InMode}}, [state](const Nda::FncValues& args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        auto pos = args.at("pos").toInt64();
        auto count = args.at("n").toInt64();
        if (self.type() != Nda::Bytes)
            return false;

        int64_t size = self.lengthOperator();
        if (pos < 0)
            pos = 0;
        if (count < 0)
            count = 0;

        NdaVariant next;
        next.initType(state->bytesType());
        for (int64_t i = 0; i < size; ++i) {
            if (i < pos || i >= pos + count)
                next.appendToBytes(self.readBytesAccess((int)i));
        }

        return self.assign(next);
    });

    // ------------------ Bytes.Chop() -----------------------------------------------------------
    state->bindPrc("bytes", "chop", {{"n", "natural", Nda::InMode}}, [state](const Nda::FncValues& args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        auto count = args.at("n").toInt64();
        if (self.type() != Nda::Bytes)
            return false;

        int64_t size = self.lengthOperator();
        int64_t newSize = count >= size ? 0 : size - count;
        auto next = bytesRange(self, state->bytesType(), 0, newSize);
        return self.assign(next);
    });

    // ------------------ Bytes.Chopped() --------------------------------------------------------
    state->bindFnc("bytes", "chopped", {{"n", "natural", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        auto count = args.at("n").toInt64();
        if (self.type() != Nda::Bytes)
            return false;

        int64_t size = self.lengthOperator();
        int64_t newSize = count >= size ? 0 : size - count;
        ret = bytesRange(self, state->bytesType(), 0, newSize);
        return true;
    });

    // ------------------ Bytes.Slice() ----------------------------------------------------------
    state->bindPrc("bytes", "slice", {{"pos", "natural", Nda::InMode}, {"n", "natural", Nda::InMode}}, [state](const Nda::FncValues& args) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Bytes)
            return false;

        auto next = bytesRange(self, state->bytesType(), args.at("pos").toInt64(), args.at("n").toInt64());
        return self.assign(next);
    });

    // ------------------ Bytes.Sliced() ---------------------------------------------------------
    state->bindFnc("bytes", "sliced", {{"pos", "natural", Nda::InMode}, {"n", "natural", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Bytes)
            return false;

        ret = bytesRange(self, state->bytesType(), args.at("pos").toInt64(), args.at("n").toInt64());
        return true;
    });

    // ------------------ Bytes.Mid() ------------------------------------------------------------
    state->bindFnc("bytes", "mid", {{"pos", "natural", Nda::InMode}, {"n", "natural", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;

        auto self = args.at("this");
        if (self.type() != Nda::Bytes)
            return false;

        ret = bytesRange(self, state->bytesType(), args.at("pos").toInt64(), args.at("n").toInt64());
        return true;
    });
}

}

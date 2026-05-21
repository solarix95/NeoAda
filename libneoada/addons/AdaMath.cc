#include "AdaMath.h"
#include "../state.h"

#include <cassert>
#include <cmath>
#include <limits>

namespace {

bool argNumber(const Nda::FncValues &args, const char *name, double &ret)
{
    bool ok;
    ret = args.at(name).toDouble(&ok);
    return ok;
}

void retNumber(NdaState *state, NdaVariant &ret, double value)
{
    ret.fromNumber(state->numberType(), value);
}

void retBool(NdaState *state, NdaVariant &ret, bool value)
{
    ret.fromBool(state->booleanType(), value);
}

using UnaryMath = double (*)(double);
using BinaryMath = double (*)(double, double);

void bindUnary(NdaState *state, const std::string &name, UnaryMath fn)
{
    state->bindFnc("math", name, {{"x", "number", Nda::InMode}}, [state, fn](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double x;
        if (!argNumber(args, "x", x))
            return false;

        retNumber(state, ret, fn(x));
        return true;
    });
}

void bindBinary(NdaState *state, const std::string &name, BinaryMath fn)
{
    state->bindFnc("math", name, {{"x", "number", Nda::InMode}, {"y", "number", Nda::InMode}}, [state, fn](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double x;
        double y;
        if (!argNumber(args, "x", x) || !argNumber(args, "y", y))
            return false;

        retNumber(state, ret, fn(x, y));
        return true;
    });
}

}

namespace Nda {

void add_AdaMath_symbols(NdaState *state)
{
    assert(state);

    state->registerType("Math", "dict");

    state->bindFnc("math", "pi", {}, [state](const Nda::FncValues&, NdaVariant &ret) -> bool {
        retNumber(state, ret, std::acos(-1.0));
        return true;
    });

    state->bindFnc("math", "e", {}, [state](const Nda::FncValues&, NdaVariant &ret) -> bool {
        retNumber(state, ret, std::exp(1.0));
        return true;
    });

    state->bindFnc("math", "tau", {}, [state](const Nda::FncValues&, NdaVariant &ret) -> bool {
        retNumber(state, ret, 2.0 * std::acos(-1.0));
        return true;
    });

    state->bindFnc("math", "infinity", {}, [state](const Nda::FncValues&, NdaVariant &ret) -> bool {
        retNumber(state, ret, std::numeric_limits<double>::infinity());
        return true;
    });

    state->bindFnc("math", "nan", {}, [state](const Nda::FncValues&, NdaVariant &ret) -> bool {
        ret.fromDoubleNan(state->numberType());
        return true;
    });

    bindUnary(state, "abs", std::fabs);
    bindUnary(state, "floor", std::floor);
    bindUnary(state, "ceil", std::ceil);
    bindUnary(state, "round", std::round);
    bindUnary(state, "trunc", std::trunc);

    bindUnary(state, "sqrt", std::sqrt);
    bindUnary(state, "cbrt", std::cbrt);
    bindBinary(state, "pow", std::pow);
    bindBinary(state, "hypot", std::hypot);
    bindBinary(state, "fmod", std::fmod);
    bindBinary(state, "remainder", std::remainder);
    bindBinary(state, "copySign", std::copysign);

    bindUnary(state, "exp", std::exp);
    bindUnary(state, "log", std::log);
    bindUnary(state, "log10", std::log10);
    bindUnary(state, "log2", std::log2);

    bindUnary(state, "sin", std::sin);
    bindUnary(state, "cos", std::cos);
    bindUnary(state, "tan", std::tan);
    bindUnary(state, "asin", std::asin);
    bindUnary(state, "acos", std::acos);
    bindUnary(state, "atan", std::atan);
    bindBinary(state, "atan2", std::atan2);

    bindUnary(state, "sinh", std::sinh);
    bindUnary(state, "cosh", std::cosh);
    bindUnary(state, "tanh", std::tanh);

    state->bindFnc("math", "min", {{"x", "number", Nda::InMode}, {"y", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double x;
        double y;
        if (!argNumber(args, "x", x) || !argNumber(args, "y", y))
            return false;

        retNumber(state, ret, std::fmin(x, y));
        return true;
    });

    state->bindFnc("math", "max", {{"x", "number", Nda::InMode}, {"y", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double x;
        double y;
        if (!argNumber(args, "x", x) || !argNumber(args, "y", y))
            return false;

        retNumber(state, ret, std::fmax(x, y));
        return true;
    });

    state->bindFnc("math", "clamp", {{"x", "number", Nda::InMode}, {"lo", "number", Nda::InMode}, {"hi", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double x;
        double lo;
        double hi;
        if (!argNumber(args, "x", x) || !argNumber(args, "lo", lo) || !argNumber(args, "hi", hi))
            return false;

        if (lo > hi) {
            const double tmp = lo;
            lo = hi;
            hi = tmp;
        }

        retNumber(state, ret, std::fmin(std::fmax(x, lo), hi));
        return true;
    });

    state->bindFnc("math", "sign", {{"x", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double x;
        if (!argNumber(args, "x", x))
            return false;

        retNumber(state, ret, (x > 0.0) - (x < 0.0));
        return true;
    });

    state->bindFnc("math", "radians", {{"degrees", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double degrees;
        if (!argNumber(args, "degrees", degrees))
            return false;

        retNumber(state, ret, degrees * std::acos(-1.0) / 180.0);
        return true;
    });

    state->bindFnc("math", "degrees", {{"radians", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double radians;
        if (!argNumber(args, "radians", radians))
            return false;

        retNumber(state, ret, radians * 180.0 / std::acos(-1.0));
        return true;
    });

    state->bindFnc("math", "isNan", {{"x", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double x;
        if (!argNumber(args, "x", x))
            return false;

        retBool(state, ret, std::isnan(x));
        return true;
    });

    state->bindFnc("math", "isFinite", {{"x", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double x;
        if (!argNumber(args, "x", x))
            return false;

        retBool(state, ret, std::isfinite(x));
        return true;
    });

    state->bindFnc("math", "isInf", {{"x", "number", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        double x;
        if (!argNumber(args, "x", x))
            return false;

        retBool(state, ret, std::isinf(x));
        return true;
    });
}

}

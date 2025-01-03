#include "variant.h"
#include "private/sharedstring.h"
#include "private/numericparser.h"
#include "private/sharedlist.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <sstream>
#include <limits>
#include <algorithm> // std::find

#define OP_SPACESHIP(v1, v2) ((int64_t)((v1) == (v2) ? 0 : ((v1) > (v2) ? +1 : -1)))
#define NDA_NAN               std::numeric_limits<double>::quiet_NaN()

bool       operator==(const NdaVariant &v1, const NdaVariant &v2);

//-------------------------------------------------------------------------------------------------
NdaVariant::NdaVariant(const Nda::RuntimeType *type)
    : mRuntimeType(type)
{
    mValue.uInt64 = 0;
}

//-------------------------------------------------------------------------------------------------
NdaVariant::NdaVariant(const NdaVariant &other)
    : mRuntimeType(other.mRuntimeType)
{
    mValue.uInt64 = 0;
    assignOther(other);
}

//-------------------------------------------------------------------------------------------------
NdaVariant::~NdaVariant()
{
    reset();
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::initType(const Nda::RuntimeType *type)
{
    if (mRuntimeType) reset();

    mRuntimeType = type;

    switch (myType()) {
    case Nda::Undefined: break;
    case Nda::Any:       break;
    case Nda::Number:       mValue.uDouble = 0.0; break;
    case Nda::Natural:      mValue.uInt64  =  0;  break;
    case Nda::Supernatural: mValue.uInt64  =  0;  break;
    case Nda::Boolean:      mValue.uByte   =  0;  break;
    case Nda::Byte:         mValue.uByte   =  0;  break;

    case Nda::String:       mValue.uPtr    =  nullptr; break;
    case Nda::List:         mValue.uPtr    =  nullptr; break;
    case Nda::Dict:         mValue.uPtr    =  nullptr; break;

    default:
        assert(0 && "not implemented");
    }
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::fromString(const Nda::RuntimeType *t, const std::string &value)
{
    assert(t);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    assert(type() == Nda::String);

    if (!value.empty())
            mValue.uPtr = new Nda::SharedString(value);
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromNumberLiteral(const Nda::RuntimeType *t, const std::string &value)
{
    assert(t);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    assert(type() == Nda::Number);

    std::string cleanLiteral = NadaNumericParser::removeSeparators(value);
    if (!NadaNumericParser::isFloatingPointLiteral(cleanLiteral))
        return false;

    mValue.uDouble = std::stod(cleanLiteral);
    return true;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::fromNumber(const Nda::RuntimeType *t, double value)
{
    assert(t);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    assert(type() == Nda::Number);
    mValue.uDouble = value;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromNaturalLiteral(const Nda::RuntimeType *t, const std::string &value)
{
    assert(t);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    assert(type() == Nda::Natural);

    std::string cleanLiteral = NadaNumericParser::removeSeparators(value);
    if (NadaNumericParser::isBasedLiteral(cleanLiteral)) {
        bool ok;
        mValue.uInt64 = NadaNumericParser::parseBasedLiteral(cleanLiteral, ok);
        if (!ok)
            return false;
        return true;
    }

    uint64_t uintValue = std::stoull(cleanLiteral, nullptr, 0);
    if (uintValue <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
        mValue.uInt64 = (static_cast<int64_t>(uintValue));
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::fromNatural(const Nda::RuntimeType *t, int64_t value)
{
    assert(t);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    assert(type() == Nda::Natural);
    mValue.uInt64 = value;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::fromSNatural(const Nda::RuntimeType *t, uint64_t value)
{
    assert(t);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    assert(type() == Nda::Supernatural);
    mValue.uUInt64 = value;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromSNaturalLiteral(const Nda::RuntimeType *t, const std::string &value)
{
    assert(t);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    assert(type() == Nda::Supernatural);

    std::string cleanLiteral = NadaNumericParser::removeSeparators(value);
    if (NadaNumericParser::isBasedLiteral(cleanLiteral)) {
        bool ok;
        mValue.uInt64 = NadaNumericParser::parseBasedLiteral(cleanLiteral, ok);
        if (!ok) {
            reset();
            return false;
        }
        return true;
    }

    try {
        mValue.uUInt64  = std::stoull(cleanLiteral, nullptr, 0);
    } catch(...) {
        reset();
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromByteLiteral(const Nda::RuntimeType *t, const std::string &value)
{
    assert(t);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    assert(type() == Nda::Byte);

    std::string cleanLiteral = NadaNumericParser::removeSeparators(value);
    if (NadaNumericParser::isBasedLiteral(cleanLiteral)) {
        bool ok;
        auto uInt64 = NadaNumericParser::parseBasedLiteral(cleanLiteral, ok);
        if (!ok)
            return false;
        if (uInt64 < 0 || uInt64 > 255)
            return false;

        mValue.uByte = (unsigned char)uInt64;
        return true;
    }

    auto uInt64  = std::stoull(cleanLiteral, nullptr, 0);
    if (uInt64 > 255)
        return false;
    mValue.uByte = (unsigned char)uInt64;
    return true;

}

//-------------------------------------------------------------------------------------------------
#if 0
bool NdaVariant::fromNumber(const std::string &value)
{
    if (mRuntimeType) reset();

    std::string cleanLiteral = NadaNumericParser::removeSeparators(value);

    try {
        if (NadaNumericParser::isBasedLiteral(cleanLiteral)) {
            bool ok;
            uint64_t value = NadaNumericParser::parseBasedLiteral(cleanLiteral, ok);
            if (!ok)
                return false;
            return fromNumber(value);
        }

        if (NadaNumericParser::isFloatingPointLiteral(cleanLiteral)) {
            double value = std::stod(cleanLiteral);
            return fromNumber(value);
        }

        uint64_t uintValue = std::stoull(cleanLiteral, nullptr, 0);
        if (uintValue <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            return fromNumber(static_cast<int64_t>(uintValue));
        } else {
            return fromNumber(uintValue);
        }
    } catch (const std::runtime_error&) {
        // std::cerr << "Fehler: " << e.what() << std::endl;
        assert(myType() == Nda::Undefined);
        return false;
    } catch (...) {
        // std::cerr << "Fehler: " << e.what() << std::endl;
        assert(myType() == Nda::Undefined);
        return false;
    }

    assert(myType() == Nda::Undefined);
    return false;
}
#endif


//-------------------------------------------------------------------------------------------------
void NdaVariant::fromDoubleNan(const Nda::RuntimeType *t)
{
    assert(t && t->dataType == Nda::Number);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    mValue.uDouble = std::numeric_limits<double>::quiet_NaN();
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::fromBool(const Nda::RuntimeType *t,bool value)
{
    assert(t && t->dataType == Nda::Boolean);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    mValue.uByte = value;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::fromReference(const Nda::RuntimeType *t, NdaVariant *other)
{
    assert(other);
    if (mRuntimeType) reset();
    mRuntimeType = t;
    mValue.uPtr  = other;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::toBool(bool *ok) const
{
    if (ok) *ok = false;
    switch (myType()) {
    case Nda::Undefined: return false; break;
    case Nda::Reference: return cInternalReference()->toBool(ok);
    case Nda::Any:       return false; break;
    case Nda::Number:    return false; break;
    case Nda::Natural:
        if (ok) *ok = true;
        return mValue.uInt64 != 0;
        break;
    case Nda::Supernatural:
        if (ok) *ok = true;
        return mValue.uInt64 != 0;
        break;
    case Nda::Boolean:
        if (ok) *ok = true;
        return mValue.uByte != 0;
        break;
    case Nda::Byte:
        if (ok) *ok = true;
        return (bool)mValue.uByte;
        break;
    case Nda::String:    return false; break;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
int64_t NdaVariant::toInt64(bool *ok) const
{
    if (ok) *ok = false;
    switch (myType()) {
    case Nda::Undefined: return 0; break;
    case Nda::Reference: return cInternalReference()->toInt64(ok);
    case Nda::Any:       return 0; break;
    case Nda::Number: {
        int ret;
        bool isInt = exact32BitInt(ret);
        if (isInt) {
            if (ok) *ok = true;
            return ret;
        }
        return 0;
    }
    case Nda::Natural:
        if (ok) *ok = true;
        return mValue.uInt64;
        break;
    case Nda::Supernatural:
        if (mValue.uInt64 >= 0) {
            if (ok) *ok = true;
            return (int64_t)mValue.uUInt64;
        }
        break;
    case Nda::Boolean:
        if (ok) *ok = true;
        return mValue.uByte;
        break;
    case Nda::Byte:
        if (ok) *ok = true;
        return (bool)mValue.uByte;
        break;

    case Nda::String:    return false; break;

    }
    return false;

}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::isNan() const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->isNan();
    return (myType() == Nda::Number) && std::isnan(mValue.uDouble);
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::assign(const NdaVariant &other)
{
    if (this == &other)
        return true;

    switch (myType()) {
    case Nda::Undefined: return false; break;
    case Nda::Reference: return internalReference()->assign(other);
        break;
    case Nda::Any: {
        assignAny(other);
        return true;
    } break;
    case Nda::Number: {
        if (other.type() == Nda::Number) {
            mValue.uDouble = other.cuValue()->uDouble;
            return true;
        }
        if (other.type() == Nda::Natural) {
            mValue.uDouble = (double)other.cuValue()->uInt64;
            return true;
        }
    } break;
    case Nda::Natural: {
        if (other.type() == Nda::Natural) {
            mValue.uInt64 = other.cuValue()->uInt64;
            return true;
        }
    } break;
    case Nda::Supernatural: {
        if (other.type() == Nda::Supernatural) {
            mValue.uUInt64 = other.cuValue()->uUInt64;
            return true;
        }
    } break;
    case Nda::Boolean: {
        if (other.type() == Nda::Boolean) {
            mValue.uByte = other.cuValue()->uByte;
            return true;
        }
    } break;
    case Nda::Byte: {
        if (other.type() == Nda::Byte) {
            mValue.uByte = other.cuValue()->uByte;
            return true;
        }
    } break;
    case Nda::String: {
        if (other.type() == Nda::String) {
            if (other.cuValue()->uPtr == mValue.uPtr)
                return true;

            reset();
            assignOtherString(other);
            return true;
        }
    } break;
    case Nda::List: {
        if (other.type() == Nda::List) {
            if (other.cInternalList() == cInternalList())
                return true;
            reset();
            assignOtherList(other);
            return true;
        }
    } break;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::equal(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->equal(other, ok);


    bool done;
    if (ok) *ok = false;

    auto res = spaceship(other, &done);

    if (!done)
        return false;

    if (std::isnan(res)) {
        if (ok) *ok = true;
        return false;
    }

    if (ok) *ok = true;
    return res == 0;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::logicalAnd(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->logicalAnd(other, ok);

    if (ok) *ok = false;

    bool done;
    bool left = toBool(&done);
    if (!done)
        return false;

    bool right = other.toBool(&done);
    if (!done)
        return false;

    if (ok) *ok = true;
    return left && right;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::logicalOr(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->logicalOr(other, ok);

    if (ok) *ok = false;

    bool done;
    bool left = toBool(&done);
    if (!done)
        return false;

    bool right = other.toBool(&done);
    if (!done)
        return false;

    if (ok) *ok = true;
    return left || right;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::logicalXor(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->logicalXor(other, ok);

    if (ok) *ok = false;

    bool done;
    bool left = toBool(&done);
    if (!done)
        return false;

    bool right = other.toBool(&done);
    if (!done)
        return false;

    if (ok) *ok = true;
    return left != right;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::greaterThen(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->greaterThen(other, ok);

    bool done;
    if (ok) *ok = false;

    auto res = spaceship(other, &done);

    if (!done)
        return false;

    if (std::isnan(res)) {
        if (ok) *ok = true;
        return false;
    }

    if (ok) *ok = true;
    return res > 0;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::lessThen(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->lessThen(other, ok);

    bool done;
    if (ok) *ok = false;

    auto res = spaceship(other, &done);

    if (!done)
        return false;

    if (std::isnan(res)) {
        if (ok) *ok = true;
        return false;
    }

    if (ok) *ok = true;
    return res < 0;
}

//-------------------------------------------------------------------------------------------------
double NdaVariant::spaceship(const NdaVariant &other, bool *ok) const
{

    if (ok) *ok = false;

    switch (myType()) {
    case Nda::Undefined: return NDA_NAN;
    case Nda::Reference: return cInternalReference()->spaceship(other, ok);
    case Nda::Any:       return NDA_NAN;
    case Nda::Number:
        if (std::isnan(mValue.uDouble) || (other.type() == Nda::Number && std::isnan(other.cuValue()->uDouble))) {
            if (ok) *ok = true;
            return NDA_NAN;
        }
        if (other.type() != myType()) {
            int v1, v2;
            bool v1IsInt = exact32BitInt(v1);
            bool v2IsInt = other.exact32BitInt(v2);
            if (v1IsInt && v2IsInt) {
                if (ok) *ok = true;
                return OP_SPACESHIP(v1,v2);
            }
            if (v2IsInt) {
                if (ok) *ok = true;
                return OP_SPACESHIP(mValue.uDouble,(double)v2);
            }
            double otherVal;
            if (other.exact64BitDbl(otherVal)) {
                if (ok) *ok = true;
                return OP_SPACESHIP(mValue.uDouble,otherVal);
            }
            return NDA_NAN; // giving up...
        }
        if (ok) *ok = true;
        return OP_SPACESHIP(mValue.uDouble,other.cuValue()->uDouble);
        break;
    case Nda::Natural:
        if (other.type() != myType())
            return NDA_NAN;
        if (ok) *ok = true;
        return OP_SPACESHIP(mValue.uInt64,other.cuValue()->uInt64);
        break;
    case Nda::Supernatural:
        if (other.type() != myType())
            return NDA_NAN;
        if (ok) *ok = true;
        return OP_SPACESHIP(mValue.uUInt64,other.cuValue()->uUInt64);
        break;
    case Nda::Boolean:
    case Nda::Byte:
        if (other.type() != myType())
            return NDA_NAN;
        if (ok) *ok = true;
        return OP_SPACESHIP(mValue.uByte,other.cuValue()->uByte);
        break;
    case Nda::String:
        if (other.type() != myType())
            return NDA_NAN;
        if (ok) *ok = true;
        if (!mValue.uPtr && !other.cuValue()->uPtr)   // both are empty
            return 0;
        else if (!mValue.uPtr)                    // I'm empty the other is not -> not greater
            return -1;
        if (!other.cuValue()->uPtr)                //  I'm not empty the other is empty -> greater
            return +1;
        else
            return OP_SPACESHIP(cInternalString()->cValue(),other.cInternalString()->cValue());
        break;
    }
    return NDA_NAN;
}


//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::concat(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->concat(other, ok);

    if (ok) *ok = false;

    switch (myType()) {
    case Nda::String: {
        NdaVariant ret;
        if (ok) *ok = true;
        if (other.type() == Nda::String)
            ret.fromString(runtimeType(), cInternalString()->cValue() + other.cInternalString()->cValue());
        else
            ret.fromString(runtimeType(), cInternalString()->cValue() + other.toString());
        return ret;
    } break;
    case Nda::List: {
        if (other.type() == Nda::List) {
            NdaVariant ret;
            ret.initType(other.runtimeType());

            const auto &myArray    = cInternalList()->cArray();
            const auto &otherArray = other.cInternalList()->cArray();
            auto &newArray = ret.internalList()->array();
            newArray = myArray;
            newArray.insert(newArray.end(), otherArray.begin(), otherArray.end());

            if (ok) *ok = true;
            return ret;
        }
    } break;
    default:
        break;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::subtract(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->subtract(other, ok);

    if (ok) *ok = false;
    if (myType()  != other.type())
        return NdaVariant();

    switch (myType()) {
    case Nda::Number: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mRuntimeType,mValue.uDouble - other.cuValue()->uDouble);
        return ret;
    } break;
    case Nda::Natural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNatural(mRuntimeType,mValue.uInt64 - other.cuValue()->uInt64);
        return ret;
    } break;
    case Nda::Supernatural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromSNatural(mRuntimeType,mValue.uUInt64 - other.cuValue()->uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::add(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->add(other, ok);

    if (ok) *ok = false;
    if (myType()  != other.type())
        return NdaVariant();

    switch (myType()) {
    case Nda::Number: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mRuntimeType, mValue.uDouble + other.cuValue()->uDouble);
        return ret;
    } break;
    case Nda::Natural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNatural(mRuntimeType, mValue.uInt64 + other.cuValue()->uInt64);
        return ret;
    } break;
    case Nda::Supernatural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromSNatural(mRuntimeType, mValue.uUInt64 + other.cuValue()->uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();

}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::modulo(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->modulo(other, ok);


    if (ok) *ok = false;
    if (myType()  != other.type())
        return NdaVariant();

    switch (myType()) {
    case Nda::Natural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNatural(mRuntimeType,mValue.uInt64 % other.cuValue()->uInt64);
        return ret;
    } break;
    case Nda::Supernatural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromSNatural(mRuntimeType,mValue.uUInt64 % other.cuValue()->uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::multiply(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->multiply(other, ok);

    if (ok) *ok = false;
    if (myType()  != other.type())
        return NdaVariant();

    switch (myType()) {
    case Nda::Number: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mRuntimeType, mValue.uDouble * other.cuValue()->uDouble);
        return ret;
    } break;
    case Nda::Natural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNatural(mRuntimeType, mValue.uInt64 * other.cuValue()->uInt64);
        return ret;
    } break;
    case Nda::Supernatural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromSNatural(mRuntimeType, mValue.uUInt64 * other.cuValue()->uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::division(const NdaVariant &other, bool *ok) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->division(other, ok);


    if (ok) *ok = false;
    if (myType()  != other.type())
        return NdaVariant();

    switch (myType()) {
    case Nda::Number: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mRuntimeType, mValue.uDouble / other.cuValue()->uDouble);
        return ret;
    } break;
    case Nda::Natural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNatural(mRuntimeType, mValue.uInt64 / other.cuValue()->uInt64);
        return ret;
    } break;
    case Nda::Supernatural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromSNatural(mRuntimeType, mValue.uUInt64 / other.cuValue()->uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();

}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::unaryOperator(const std::string &op, bool *ok) const
{
    if (myType() == Nda::Reference) {
        return cInternalReference()->unaryOperator(op, ok);
    }

    if (ok) *ok = false;

    NdaVariant ret;
    switch (myType()) {
    case Nda::Undefined: break;
    case Nda::Any:       break;
    case Nda::Number: {
        if (op == "+") {
            if (ok) *ok = true;
            return *this;
        }
        if (op == "-") {
            // FIXME: nan? inf?
            if (ok) *ok = true;
            ret.fromNumber(mRuntimeType, -mValue.uDouble);
        }
    } break;
    case Nda::Natural:{
        if (op == "+") {
            if (ok) *ok = true;
            return *this;
        }
        if (op == "-") {
            if (ok) *ok = true;
            ret.fromNatural(mRuntimeType, -mValue.uInt64);
        }
    } break;
    case Nda::Supernatural:{
        if (op == "+") {
            if (ok) *ok = true;
            return *this;
        }
    } break;
    case Nda::Boolean:
        break;
    case Nda::Byte:
        break;
    case Nda::String:
        break;
    }

    return ret;
}

//-------------------------------------------------------------------------------------------------
int NdaVariant::lengthOperator() const
{
    if (myType() == Nda::Reference) {
        return cInternalReference()->lengthOperator();
    }

    int length = 0;
    switch (myType()) {
    case Nda::Reference: break;
    case Nda::Undefined: break;
    case Nda::Any:
    case Nda::Number:
    case Nda::Natural:
    case Nda::Supernatural:
    case Nda::Boolean:
    case Nda::Byte:
        length = 1;
        break;
    case Nda::String:
        length = cInternalString() ? cInternalString()->cValue().length() : 0;
        break;
    case Nda::List:
        length = listSize();
        break;
    case Nda::Dict:
        assert(0 && "not implemented");
        break;
    }

    return length;
}

//-------------------------------------------------------------------------------------------------
int NdaVariant::listSize() const
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return cInternalReference()->listSize();

    if (!mValue.uPtr)
        return 0;

    return cInternalList()->cArray().size();
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::appendToList(const NdaVariant &value)
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return internalReference()->appendToList(value);

    if (!mValue.uPtr)
        mValue.uPtr = new Nda::SharedList();
    else
        detachList();

    internalList()->array().push_back(value);
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::insertIntoList(int index, const NdaVariant &value)
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return internalReference()->insertIntoList(index, value);

    if (!mValue.uPtr)
        mValue.uPtr = new Nda::SharedList();
    else
        detachList();

    auto &array = internalList()->array();

    if (index >= (int)array.size())
        appendToList(value);

    if (index < 0)
        index = 0;

    array.insert(array.begin() + index,value);
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::takeFromList(int index)
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return internalReference()->takeFromList(index);

    if (index < 0)
        return;

    if (index >= listSize())
        return;

    detachList();

    auto &array = internalList()->array();
    array.erase(array.begin() + index);
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaVariant::writeAccess(int index)
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return internalReference()->writeAccess(index);

    assert(index >= 0);
    assert(index < listSize());

    detachList();
    auto &array = internalList()->array();
    return array.at(index);
}

//-------------------------------------------------------------------------------------------------
const NdaVariant &NdaVariant::readAccess(int index) const
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return cInternalReference()->readAccess(index);

    assert(index >= 0);
    assert(index < listSize());

    const auto &array = cInternalList()->cArray();
    return array.at(index);
}

//-------------------------------------------------------------------------------------------------
int NdaVariant::indexInList(const NdaVariant &value) const
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return cInternalReference()->indexInList(value);

    if (listSize() <= 0)
        return -1;

    const auto &array = cInternalList()->cArray();
    auto pos = std::find(array.begin(), array.end(), value);
    if (pos == array.end())
        return -1;

    return std::distance(array.begin(), pos);
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::containsInList(const NdaVariant &value) const
{
    return indexInList(value) >= 0;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::reverseList()
{
    if (myType() == Nda::Reference)
        return internalReference()->reverseList();

    assert(type() == Nda::List);

    if (listSize() <= 1)
        return;

    detachList();
    auto &array = internalList()->array();
    std::reverse(array.begin(), array.end());
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::clearList()
{
    if (myType() == Nda::Reference)
        return internalReference()->clearList();

    assert(type() == Nda::List);

    if (listSize() <= 0)
        return;

    internalList()->releaseRef();
    mValue.uPtr = nullptr;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::assignOther(const NdaVariant &other)
{
    if (mRuntimeType) reset();

    switch (other.myType()) {
    case Nda::Undefined: return;
    case Nda::Reference: {
        mRuntimeType = other.mRuntimeType;
        mValue.uPtr  = other.mValue.uPtr;
        return;
    } break;
    case Nda::Any:       return;
    case Nda::Number:
    case Nda::Natural:
    case Nda::Supernatural:
    case Nda::Boolean:
    case Nda::Byte: {
        mRuntimeType  = other.mRuntimeType;
        mValue.uInt64 = other.mValue.uInt64;
        return;
    } break;
    case Nda::String:
        assignOtherString(other);
        return;
    case Nda::List:
        assignOtherList(other);
        return;
    }
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::assignAny(const NdaVariant &other)
{
    switch (other.type()) {
    case Nda::Undefined: return;
    case Nda::Reference: return;
    case Nda::Any:       return;
    case Nda::Number:
    case Nda::Natural:
    case Nda::Supernatural:
    case Nda::Boolean:
    case Nda::Byte: {
        mRuntimeType  = other.runtimeType();
        mValue.uInt64 = other.cuValue()->uUInt64;
        return;
    } break;
    case Nda::String:
        reset();
        assignOtherString(other);
        return;
    case Nda::List:
        reset();
        assignOtherList(other);
        return;
    case Nda::Dict:
        reset();
        assert(0);
    return;
}
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::assignOtherString(const NdaVariant &other)
{
    assert(myType()     == Nda::Undefined);
    assert(other.type() == Nda::String);

    mRuntimeType = other.runtimeType(); // set my type to "string"
    assert(type() == Nda::String);

    if (!other.cuValue()->uPtr)
        return;

    mValue.uPtr = other.cuValue()->uPtr;
    internalString()->addRef();
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::assignOtherList(const NdaVariant &other)
{
    assert(myType()    == Nda::Undefined);
    assert(mValue.uPtr == nullptr);

    mRuntimeType = other.runtimeType();
    assert(type() == Nda::List);

    if (!other.mValue.uPtr)
        return;
    mValue.uPtr = other.cuValue()->uPtr;
    internalList()->addRef();
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::setString(const std::string &newValue)
{
    if (myType() == Nda::Reference)
        return internalReference()->setString(newValue);

    if (myType() != Nda::String)
        return false;
    if (newValue.empty() && !mValue.uPtr)  // nothing to to
        return true;
    if (newValue.empty() &&  mValue.uPtr){ // almost nothing to to
        internalString()->releaseRef();
        mValue.uPtr = nullptr;
        return true;
    }

    // same value.. nothing to to
    if (mValue.uPtr && (newValue == internalString()->cValue()))
        return true;

    if (mValue.uPtr) {
        if (internalString()->refCount() > 1) { // detach?
            internalString()->releaseRef();
            mValue.uPtr = new Nda::SharedString(newValue);
            return true;
        }
        internalString()->value() = newValue;
    } else {
        mValue.uPtr = new Nda::SharedString(newValue);
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
std::string NdaVariant::toString() const
{
    std::ostringstream oss;

    switch (myType()) {
    case Nda::Undefined: return "";
    case Nda::Reference: return cInternalReference()->toString();
    case Nda::Any:       return "";
    case Nda::Number:
        oss << std::setprecision(15) << std::fixed << mValue.uDouble;
        break;
    case Nda::Natural:
        oss << mValue.uInt64;
        break;
    case Nda::Supernatural:
        oss << mValue.uUInt64;
        break;
    case Nda::Boolean:
    case Nda::Byte:
    case Nda::String:
        if (mValue.uPtr)
            return cInternalString()->cValue();
        return "";
    case Nda::List:
        if (mValue.uPtr) {
            std::string ret;
            for (const auto &v : cInternalList()->cArray()) {
                if (!ret.empty())
                    ret = ret + ",";
                ret += v.toString();
            }
            ret = "[" + ret + "]";
            return ret;
        }

        return "[]";
    }

    return oss.str();
}

//-------------------------------------------------------------------------------------------------
Nda::Type NdaVariant::type() const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->type();

    if (mRuntimeType)
        return mRuntimeType->dataType;

    return Nda::Undefined;
}

//-------------------------------------------------------------------------------------------------
const Nda::RuntimeType *NdaVariant::runtimeType() const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->runtimeType();
    return mRuntimeType;
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaVariant::operator=(const NdaVariant &other)
{
    assignOther(other);
    return *this;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::dereference()
{
    while (myType() == Nda::Reference)
        assignOther(*internalReference());
}

//-------------------------------------------------------------------------------------------------
int NdaVariant::refCount() const
{
    if (!mValue.uPtr)
        return 0;

    if (myType() == Nda::String)
        return cInternalString()->refCount();

    assert(0 && "Not Implemented");
    return 0;
}

//-------------------------------------------------------------------------------------------------
Nda::Type NdaVariant::numericType(const std::string &literal)
{
    assert(!literal.empty());
    auto lastChar = literal.back();

    std::string cleanLiteral = NadaNumericParser::removeSeparators(literal);

    if (cleanLiteral.empty())
        return Nda::Undefined;

    if (lastChar == 'n')
        return Nda::Natural;

    if (lastChar == 'u')
        return Nda::Supernatural;

    if (lastChar == 'd')
        return Nda::Number;

    if (lastChar == 'b')
        return Nda::Byte;

    if (!std::isdigit(lastChar) && (lastChar != '#'))
        return Nda::Undefined;

    // no explizit typing -> default guessing:

    if (NadaNumericParser::isFloatingPointLiteral(cleanLiteral))
        return Nda::Number;

    if (NadaNumericParser::isBasedLiteral(cleanLiteral))
        return Nda::Natural;

    try {

        uint64_t uintValue = std::stoull(cleanLiteral, nullptr, 0);
        if (uintValue <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            return Nda::Natural;
        } else {
            return Nda::Supernatural;
        }
    } catch (const std::runtime_error&) {
        return Nda::Undefined;
    } catch (...) {
        return Nda::Undefined;
    }

    return Nda::Undefined;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromNumber(const std::string &value, int64_t &ret)
{
    std::string cleanLiteral = NadaNumericParser::removeSeparators(value);

    try {
        ret = std::stoll(cleanLiteral, nullptr, 0);
    } catch (...) {
        return false;
    }

    return true;
}


//-------------------------------------------------------------------------------------------------
void NdaVariant::reset()
{
    switch (myType()) {
    case Nda::Undefined: return;
    case Nda::Any:
    case Nda::Number:
    case Nda::Natural:
    case Nda::Supernatural:
    case Nda::Boolean:
    case Nda::Byte:
        mValue.uInt64 = 0;
        break;
    case Nda::String:
        if (mValue.uPtr)
            ((Nda::SharedString*)mValue.uPtr)->releaseRef();
        mValue.uPtr = nullptr;
        break;
    case Nda::Reference:
        mValue.uPtr = nullptr;
        break;
    case Nda::List:
        if (mValue.uPtr)
            ((Nda::SharedList*)mValue.uPtr)->releaseRef();
        mValue.uPtr = nullptr;
        break;
    }

    mRuntimeType = nullptr;
}

//-------------------------------------------------------------------------------------------------
Nda::SharedString *NdaVariant::internalString()
{
    assert(myType() == Nda::String);
    assert(mValue.uPtr);
    return ((Nda::SharedString*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
const Nda::SharedString *NdaVariant::cInternalString() const
{
    assert(myType() == Nda::String);
    return ((Nda::SharedString*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
NdaVariant *NdaVariant::internalReference()
{
    assert(myType() == Nda::Reference);
    assert(mValue.uPtr);
    return ((NdaVariant*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
const NdaVariant *NdaVariant::cInternalReference() const
{
    assert(myType() == Nda::Reference);
    assert(mValue.uPtr);
    return ((NdaVariant*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
Nda::SharedList *NdaVariant::internalList()
{
    if (myType() == Nda::Reference)
        return internalReference()->internalList();

    assert(myType() == Nda::List);
    if (!mValue.uPtr)
        mValue.uPtr = new Nda::SharedList();
    return ((Nda::SharedList*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
const Nda::SharedList   *NdaVariant::cInternalList() const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->cInternalList();

    assert(myType() == Nda::List);
    return ((Nda::SharedList*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::detachList()
{
    assert(myType() == Nda::List);
    assert(mValue.uPtr);
    if (internalList()->refCount() <= 1)
        return;

    auto *newList = new Nda::SharedList();
    newList->array() = internalList()->array(); // deep copy
    internalList()->releaseRef();
    mValue.uPtr = newList;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::exact32BitInt(int &value) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->exact32BitInt(value);

    if ((myType() == Nda::Natural) && (mValue.uInt64 <= INT32_MAX) && (mValue.uInt64 >= INT32_MIN))
    {
        value = (int)mValue.uInt64;
        return true;
    }

    if ((myType() == Nda::Supernatural) && (mValue.uUInt64 <= INT32_MAX))
    {
        value = (int)mValue.uUInt64;
        return true;
    }

    if ((myType() == Nda::Number) && (std::floor(mValue.uDouble) == mValue.uDouble)
        && (mValue.uDouble >= INT32_MIN && mValue.uDouble <= INT32_MAX))
    {
        value = static_cast<int>(mValue.uDouble);
        return true;
    }

    if (myType() == Nda::Byte) {
        value = mValue.uByte;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::exact64BitDbl(double &value) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->exact64BitDbl(value);

    switch (myType()) {
    case Nda::Undefined:
    case Nda::Reference:
    case Nda::Any:
    case Nda::String:
        return false;

    case Nda::Number: {
        value = mValue.uDouble;
        return true;
    } break;
    case Nda::Natural: {
        if (mValue.uInt64 >= -((int64_t(1) << 53) - 1) && mValue.uInt64 <= ((int64_t(1) << 53) - 1))
            value = static_cast<double>(mValue.uInt64);
        return true;
    } break;

    case Nda::Supernatural: {
        if (mValue.uInt64 >= 0 && mValue.uInt64 <= ((int64_t(1) << 53) - 1))
            value = static_cast<double>(mValue.uUInt64);
        return true;
    } break;
    case Nda::Boolean:
    case Nda::Byte:
        value = static_cast<double>(mValue.uByte);
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//                                   local functions
//-------------------------------------------------------------------------------------------------

bool       operator==(const NdaVariant &v1, const NdaVariant &v2)
{
    if (v1.type() != v2.type())
        return false;

    return v1.equal(v2);
}

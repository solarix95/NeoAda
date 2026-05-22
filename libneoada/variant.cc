#include "variant.h"
#include "private/sharedstring.h"
#include "private/numericparser.h"
#include "private/sharedlist.h"
#include "private/sharedbytes.h"
#include "private/shareddict.h"

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
bool nadaParseDoubleLiteral(const std::string &s, double &out)
{
    std::istringstream in(s);
    in.imbue(std::locale::classic());

    in >> out;
    return in && in.eof();
}


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
bool NdaVariant::operator<(const NdaVariant &other) const
{
    bool done;
    bool lt = lessThen(other, &done);
    if (!done)
        return this->toString() < other.toString();
    return lt;
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

    case Nda::String:       mValue.uPtr    =  new Nda::SharedString(); break;
    case Nda::List:         mValue.uPtr    =  new Nda::SharedList();   break;
    case Nda::Bytes:        mValue.uPtr    =  new Nda::SharedBytes();  break;
    case Nda::Dict:         mValue.uPtr    =  new Nda::SharedDict();   break;

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

    long long i32;
    if (NadaNumericParser::isInteger32(cleanLiteral, &i32)) {
        mValue.uDouble = (int32_t)i32;
        return true;
    }
    if (!NadaNumericParser::isFloatingPointLiteral(cleanLiteral))
        return false;

    double parsed = 0.0;
    if (!nadaParseDoubleLiteral(cleanLiteral, parsed))
        return false;

    mValue.uDouble = parsed;
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

    long long ivalue;
    if (!NadaNumericParser::isInteger32(cleanLiteral,&ivalue))
        return false;

    if (ivalue < 0)
        return false;
    if (ivalue > 255)
        return false;

    mValue.uByte = (unsigned char)ivalue;
    return true;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::fromByte(const Nda::RuntimeType *t, unsigned char value)
{
    assert(t);
    if (mRuntimeType) reset();

    mRuntimeType = t;
    assert(type() == Nda::Byte);
    mValue.uByte = value;
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
    case Nda::Bytes:     return false; break;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
double NdaVariant::toDouble(bool *ok) const
{
    if (ok) *ok = false;
    switch (myType()) {
    case Nda::Undefined: return 0; break;
    case Nda::Reference: return cInternalReference()->toDouble(ok);
    case Nda::Any:       return 0; break;
    case Nda::Number:
        if (ok) *ok = true;
        return mValue.uDouble;
        break;
    case Nda::Natural:
        if (ok) *ok = true;
        return (double)mValue.uInt64;
        break;
    case Nda::Supernatural:
        if (ok) *ok = true;
        return (double)mValue.uUInt64;
        break;
    case Nda::Boolean:
        if (ok) *ok = true;
        return (double)mValue.uByte;
        break;
    case Nda::Byte:
        if (ok) *ok = true;
        return (double)mValue.uByte;
        break;
    default:
        return 0.0; break;
    }
    return 0.0;
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
        if (mValue.uInt64 >= 0) { // test unsigned overflow
            if (ok) *ok = true;
            return (int64_t)mValue.uUInt64;
        }
        break;
    case Nda::Boolean:
        if (ok) *ok = true;
        return (bool)mValue.uByte;
        break;
    case Nda::Byte:
        if (ok) *ok = true;
        return mValue.uByte;
        break;

    case Nda::String:    return false; break;
    case Nda::Bytes:     return false; break;

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
        if (other.type() == Nda::Natural || other.type() == Nda::Byte) {
            mValue.uInt64 = other.cuValue()->uInt64;
            return true;
        }
    } break;
    case Nda::Supernatural: {
        if (other.type() == Nda::Supernatural || other.type() == Nda::Byte) {
            mValue.uUInt64 = other.cuValue()->uUInt64;
            return true;
        }
        if (other.type() == Nda::Natural) {
            if (other.cuValue()->uInt64 >= 0) {
                mValue.uUInt64 = other.cuValue()->uInt64;
                return true;
            }
        }
        if (other.type() == Nda::Number) {
            if (other.cuValue()->uDouble >= 0) {
                mValue.uUInt64 = other.cuValue()->uDouble;
                return true;
            }
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
    case Nda::Bytes: {
        if (other.type() == Nda::Bytes) {
            if (other.cInternalBytes() == cInternalBytes())
                return true;
            reset();
            assignOtherBytes(other);
            return true;
        }
    } break;
    case Nda::Dict: {
        if (other.type() == Nda::Dict) {
            if (other.cInternalDict() == cInternalDict())
                return true;
            reset();
            assignOtherDict(other);
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

    // double camparison
    if (type() != other.type() && (type() == Nda::Number || other.type() == Nda::Number)) {
        bool vok = true;
        double v1 = toDouble(&vok);
        if (!vok)
            return NDA_NAN;
        double v2 = other.toDouble(&vok);
        if (!vok)
            return NDA_NAN;

        // double spaceship ok..
        if (ok) *ok = true;
        if (std::isnan(v1) || std::isnan(v2)) {
            return NDA_NAN;
        }
        return OP_SPACESHIP(v1,v2);
    }

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
        if (other.type() != myType()) {
            if (other.type() == Nda::Supernatural) {
                if (ok) *ok = true;
                if (mValue.uInt64 < 0)
                    return -1;
                return OP_SPACESHIP(static_cast<uint64_t>(mValue.uInt64), other.cuValue()->uUInt64);
            }

            int v2;
            bool v2IsInt = other.exact32BitInt(v2);
            if (v2IsInt) {
                if (ok) *ok = true;
                return OP_SPACESHIP(mValue.uInt64,v2);
            }
            return NDA_NAN; // giving up...
        }
        if (ok) *ok = true;
        return OP_SPACESHIP(mValue.uInt64,other.cuValue()->uInt64);
        break;
    case Nda::Supernatural:
        if (other.type() != myType()) {
            int v2;
            bool v2IsInt = other.exact32BitInt(v2);
            if (v2IsInt) {
                if (ok) *ok = true;
                if (v2 < 0)
                    return 1;
                return OP_SPACESHIP(mValue.uUInt64,(unsigned)v2);
            }
            return NDA_NAN; // giving up...
        }
        if (ok) *ok = true;
        return OP_SPACESHIP(mValue.uUInt64,other.cuValue()->uUInt64);
        break;
    case Nda::Boolean:
    case Nda::Byte:
        if (other.type() != myType()) {
            if (other.type() == Nda::Supernatural) {
                if (ok) *ok = true;
                return OP_SPACESHIP(static_cast<uint64_t>(mValue.uByte), other.cuValue()->uUInt64);
            }

            int v2;
            bool v2IsInt = other.exact32BitInt(v2);
            if (v2IsInt) {
                if (ok) *ok = true;
                return OP_SPACESHIP((int)mValue.uByte,v2);
            }
            return NDA_NAN; // giving up...
        }
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

        ret.fromString(runtimeType(), toString() + other.toString());

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

    if ((type() == Nda::Number) || (other.type() == Nda::Number))
        return doubleSubtraction(other,  ok);

    switch (myType()) {
    case Nda::Natural: {
        bool isLong;
        auto offset = other.toInt64(&isLong);
        if (!isLong)
            return NdaVariant();
        if ((offset > 0 && mValue.uInt64 < std::numeric_limits<int64_t>::min() + offset) ||
            (offset < 0 && mValue.uInt64 > std::numeric_limits<int64_t>::max() + offset))
            return NdaVariant();

        NdaVariant ret;
        ret.fromNatural(mRuntimeType, mValue.uInt64 - offset);
        if (ok) *ok = true;
        return ret;
    } break;
    case Nda::Supernatural: {

        uint64_t offset;
        if (other.type() == Nda::Supernatural) {
            offset = other.cuValue()->uUInt64;
        } else {
            bool isLong;
            auto intOffset = other.toInt64(&isLong);
            if (!isLong || intOffset < 0)
                return NdaVariant();
            offset = intOffset;
        }
        if (offset > mValue.uUInt64)
            return NdaVariant();

        NdaVariant ret;
        ret.fromSNatural(mRuntimeType, mValue.uUInt64 - offset);
        if (ok) *ok = true;
        return ret;
    } break;
    case Nda::Byte: {

        bool isLong;
        auto intOffset = other.toInt64(&isLong);
        if (!isLong || intOffset < 0 || intOffset > mValue.uByte)
            return NdaVariant();

        NdaVariant ret;
        ret.fromByte(mRuntimeType, mValue.uByte - intOffset);
        if (ok) *ok = true;
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

    if ((type() == Nda::Number) || (other.type() == Nda::Number))
        return doubleAddition(other,  ok);

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

        uint64_t offset;
        if (other.type() == Nda::Supernatural) {
            offset = other.cuValue()->uUInt64;
        } else {
            bool isLong;
            auto intOffset = other.toInt64(&isLong);
            if (!isLong || intOffset < 0)
                return NdaVariant();
            offset = intOffset;
        }

        NdaVariant ret;
        ret.fromSNatural(mRuntimeType, mValue.uUInt64 + offset);
        if (ok) *ok = true;
        return ret;
    } break;
    case Nda::Byte: {

        bool isLong;
        auto intOffset = other.toInt64(&isLong);
        if (!isLong || intOffset < 0 || intOffset > 255)
            return NdaVariant();

        NdaVariant ret;
        ret.fromByte(mRuntimeType, mValue.uByte + intOffset);
        if (ok) *ok = true;
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
    if ((type() == Nda::Number) || (other.type() == Nda::Number))
        return NdaVariant();

    switch (myType()) {
    case Nda::Natural: {
        bool isLong;
        auto divisor = other.toInt64(&isLong);
        if (!isLong)
            return NdaVariant();

        NdaVariant ret;
        ret.fromNatural(mRuntimeType, mValue.uInt64 % divisor);
        if (ok) *ok = true;
        return ret;
    } break;
    case Nda::Supernatural: {

        uint64_t divisor;
        if (other.type() == Nda::Supernatural) {
            divisor = other.cuValue()->uUInt64;
        } else {
            bool isLong;
            auto intDivisor = other.toInt64(&isLong);
            if (!isLong || intDivisor < 0)
                return NdaVariant();
            divisor = intDivisor;
        }

        NdaVariant ret;
        ret.fromSNatural(mRuntimeType, mValue.uUInt64 % divisor);
        if (ok) *ok = true;
        return ret;
    } break;
    case Nda::Byte: {

        bool isLong;
        auto intDivisor = other.toInt64(&isLong);
        if (!isLong || intDivisor < 0 || intDivisor > 255)
            return NdaVariant();

        NdaVariant ret;
        ret.fromByte(mRuntimeType, mValue.uByte % intDivisor);
        if (ok) *ok = true;
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

    if ((type() == Nda::Number) || (other.type() == Nda::Number))
        return doubleMultiply(other, ok);

    switch (myType()) {
    case Nda::Natural: {
        bool isLong;
        auto factor = other.toInt64(&isLong);
        if (!isLong)
            return NdaVariant();

        NdaVariant ret;
        ret.fromNatural(mRuntimeType, mValue.uInt64 * factor);
        if (ok) *ok = true;
        return ret;
    } break;
    case Nda::Supernatural: {

        uint64_t factor;
        if (other.type() == Nda::Supernatural) {
            factor = other.cuValue()->uUInt64;
        } else {
            bool isLong;
            auto intFactor = other.toInt64(&isLong);
            if (!isLong || intFactor < 0)
                return NdaVariant();
            factor = intFactor;
        }

        NdaVariant ret;
        ret.fromSNatural(mRuntimeType, mValue.uUInt64 * factor);
        if (ok) *ok = true;
        return ret;
    } break;
    case Nda::Byte: {

        bool isLong;
        auto intFactor = other.toInt64(&isLong);
        if (!isLong || intFactor < 0 || intFactor > 255)
            return NdaVariant();

        NdaVariant ret;
        ret.fromByte(mRuntimeType, mValue.uByte * intFactor);
        if (ok) *ok = true;
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::division(const NdaVariant &other, bool &dbz, bool *ok) const
{
    dbz = false;
    if (myType() == Nda::Reference)
        return cInternalReference()->division(other, dbz, ok);

    if (ok) *ok = false;

    if ((type() == Nda::Number) || (other.type() == Nda::Number))
        return doubleDivision(other, dbz, ok);

    switch (myType()) {
    case Nda::Natural: {
        bool isLong;
        auto divisor = other.toInt64(&isLong);
        if (!isLong)
            return NdaVariant();

        if (divisor == 0) {
            dbz = true;
            return NdaVariant();
        }
        NdaVariant ret;
        ret.fromNatural(mRuntimeType, mValue.uInt64 / divisor);
        if (ok) *ok = true;
        return ret;
    } break;
    case Nda::Supernatural: {

        uint64_t divisor;
        if (other.type() == Nda::Supernatural) {
            divisor = other.cuValue()->uUInt64;
        } else {
            bool isLong;
            auto intDivisor = other.toInt64(&isLong);
            if (!isLong || intDivisor < 0)
                return NdaVariant();
            divisor = intDivisor;
        }

        if (divisor == 0) {
            dbz = true;
            return NdaVariant();
        }

        NdaVariant ret;
        ret.fromSNatural(mRuntimeType, mValue.uUInt64 / divisor);
        if (ok) *ok = true;
        return ret;
    } break;
    case Nda::Byte: {

        bool isLong;
        auto intDivisor = other.toInt64(&isLong);
        if (!isLong)
            return NdaVariant();
        if (intDivisor < 0 ||
            intDivisor > 255)
        {
            return NdaVariant();
        }
        if (intDivisor == 0) {
            dbz = true;
            return NdaVariant();
        }

        NdaVariant ret;
        ret.fromByte(mRuntimeType, mValue.uByte / intDivisor);
        if (ok) *ok = true;
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
        length = cInternalList() ? cInternalList()->cArray().size() : 0;
        break;
    case Nda::Bytes:
        length = cInternalBytes() ? cInternalBytes()->cArray().size() : 0;
        break;
    case Nda::Dict:
        length = cInternalDict() ? cInternalDict()->cDict().size() : 0;
        break;
    }

    return length;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::appendToList(const NdaVariant &value)
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return internalReference()->appendToList(value);

    assert(mValue.uPtr);
    detachList();
    internalList()->array().push_back(value);
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::insertIntoList(int index, const NdaVariant &value)
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return internalReference()->insertIntoList(index, value);

    assert(mValue.uPtr);
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

    if (index >= lengthOperator())
        return;

    detachList();

    auto &array = internalList()->array();
    array.erase(array.begin() + index);
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaVariant::writeListAccess(int index)
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return internalReference()->writeListAccess(index);

    assert(index >= 0);
    assert(index < lengthOperator());

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
    assert(index < lengthOperator());

    const auto &array = cInternalList()->cArray();
    return array.at(index);
}

//-------------------------------------------------------------------------------------------------
int NdaVariant::indexInList(const NdaVariant &value) const
{
    assert(type() == Nda::List);
    if (myType() == Nda::Reference)
        return cInternalReference()->indexInList(value);

    if (lengthOperator() <= 0)
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

    if (lengthOperator() <= 1)
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

    if (lengthOperator() <= 0)
        return;

    assert(mValue.uPtr);
    detachList();
    internalList()->array().clear();
}


//-------------------------------------------------------------------------------------------------
bool NdaVariant::appendToBytes(const NdaVariant &value)
{
    if (myType() == Nda::Reference)
        return internalReference()->appendToBytes(value);

    assert(type() == Nda::Bytes);
    if (value.type() != Nda::Byte)
        return false;

    assert(mValue.uPtr);
    detachBytes();
    internalBytes()->array().push_back(value);
    return true;
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaVariant::writeBytesAccess(int index)
{
    if (myType() == Nda::Reference)
        return internalReference()->writeBytesAccess(index);

    assert(type() == Nda::Bytes);
    assert(index >= 0);
    assert(index < lengthOperator());

    detachBytes();
    return internalBytes()->array()[index];
}

//-------------------------------------------------------------------------------------------------
const NdaVariant &NdaVariant::readBytesAccess(int index) const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->readBytesAccess(index);

    assert(type() == Nda::Bytes);
    assert(index >= 0);
    assert(index < lengthOperator());

    return cInternalBytes()->cArray()[index];
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::clearBytes()
{
    if (myType() == Nda::Reference)
        return internalReference()->clearBytes();

    assert(type() == Nda::Bytes);

    if (lengthOperator() <= 0)
        return;

    assert(mValue.uPtr);
    detachBytes();
    internalBytes()->array().clear();
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::appendToDict(const NdaVariant &key, const NdaVariant &value)
{
    assert(type() == Nda::Dict);
    if (myType() == Nda::Reference)
        return internalReference()->appendToDict(key,value);

    assert(mValue.uPtr);
    detachDict();
    internalDict()->dict()[key] = value;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::contains(const NdaVariant &key) const
{
    assert(type() == Nda::Dict);
    if (myType() == Nda::Reference)
        return cInternalReference()->contains(key);
    assert(mValue.uPtr);
    return cInternalDict()->cDict().count(key) > 0;
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaVariant::writeDictAccess(const NdaVariant &key)
{
    assert(type() == Nda::Dict);
    if (myType() == Nda::Reference)
        return internalReference()->writeDictAccess(key);

    assert(mValue.uPtr);
    detachDict();

    return internalDict()->dict()[key]; // read only: internalDict()->dict().at(key);
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::takeFromDict(const NdaVariant &key)
{
    assert(type() == Nda::Dict);
    if (myType() == Nda::Reference)
        return internalReference()->takeFromDict(key);

    if (!contains(key))
        return;

    detachDict();

    internalDict()->dict().erase(key);
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
    case Nda::Bytes:
        assignOtherBytes(other);
        return;
    case Nda::Dict:
        assignOtherDict(other);
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
    case Nda::Bytes:
        reset();
        assignOtherBytes(other);
        return;
    case Nda::Dict:
        reset();
        assignOtherDict(other);
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
void NdaVariant::assignOtherBytes(const NdaVariant &other)
{
    assert(myType()    == Nda::Undefined);
    assert(mValue.uPtr == nullptr);

    mRuntimeType = other.runtimeType();
    assert(type() == Nda::Bytes);

    if (!other.mValue.uPtr)
        return;
    mValue.uPtr = other.cuValue()->uPtr;
    internalBytes()->addRef();
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::assignOtherDict(const NdaVariant &other)
{
    assert(myType()    == Nda::Undefined);
    assert(mValue.uPtr == nullptr);

    mRuntimeType = other.runtimeType();
    assert(type() == Nda::Dict);

    if (!other.mValue.uPtr)
        return;
    mValue.uPtr = other.cuValue()->uPtr;
    internalDict()->addRef();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::doubleAddition(const NdaVariant &other, bool *ok) const
{
    bool isDouble;
    auto offset = other.toDouble(&isDouble);
    if (!isDouble)
        return NdaVariant();

    auto value = toDouble(ok);

    NdaVariant ret;
    ret.fromNumber(type() == Nda::Number ? runtimeType() : other.runtimeType(), value + offset);

    if (ok) *ok = true;
    return ret;

}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::doubleSubtraction(const NdaVariant &other, bool *ok) const
{
    bool isDouble;
    auto offset = other.toDouble(&isDouble);
    if (!isDouble)
        return NdaVariant();

    auto value = toDouble(ok);

    NdaVariant ret;
    ret.fromNumber(type() == Nda::Number ? runtimeType() : other.runtimeType(), value - offset);

    if (ok) *ok = true;
    return ret;

}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::doubleDivision(const NdaVariant &other, bool &dbz, bool *ok) const
{
    bool isDouble;
    auto divisor = other.toDouble(&isDouble);
    if (!isDouble)
        return NdaVariant();

    if (divisor == 0) {
        dbz = true;
        return NdaVariant();
    }

    auto value = toDouble(ok);

    NdaVariant ret;
    ret.fromNumber(type() == Nda::Number ? runtimeType() : other.runtimeType(), value/divisor);

    if (ok) *ok = true;
    return ret;
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::doubleMultiply(const NdaVariant &other, bool *ok) const
{
    bool isDouble;
    auto divisor = other.toDouble(&isDouble);
    if (!isDouble)
        return NdaVariant();

    auto value = toDouble(ok);

    NdaVariant ret;
    ret.fromNumber(type() == Nda::Number ? runtimeType() : other.runtimeType(), value*divisor);

    if (ok) *ok = true;
    return ret;
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
        return mValue.uByte ? "true" : "false";
    case Nda::Byte:
        oss << (int)mValue.uByte;
        break;
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
    case Nda::Bytes:
        if (mValue.uPtr) {
            std::string ret;
            for (const auto &v : cInternalBytes()->cArray()) {
                if (!ret.empty())
                    ret = ret + ",";
                ret += v.toString();
            }
            ret = "Bytes[" + ret + "]";
            return ret;
        }

        return "Bytes[]";
    case Nda::Dict:
        if (mValue.uPtr) {
            std::string ret;

            for (const auto &v : cInternalDict()->cDict()) {
                if (!ret.empty())
                    ret = ret + ",";
                ret += v.first.toString() + ":" + v.second.toString();
            }
            ret = "{" + ret + "}";
            return ret;
        }

        return "{}";
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

    // TODO: one for all.. cInternalSharedObject?
    if (myType() == Nda::String)
        return cInternalString()->refCount();

    if (myType() == Nda::List)
        return cInternalList()->refCount();

    if (myType() == Nda::Bytes)
        return cInternalBytes()->refCount();

    if (myType() == Nda::Dict)
        return cInternalDict()->refCount();

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
    case Nda::Reference:
        mValue.uPtr = nullptr;
        break;
    case Nda::String:
    case Nda::List:
    case Nda::Bytes:
    case Nda::Dict:
        if (mValue.uPtr) {
            internalSharedObject()->releaseRef();
            mValue.uPtr = nullptr;
        }
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
Nda::SharedBytes *NdaVariant::internalBytes()
{
    if (myType() == Nda::Reference)
        return internalReference()->internalBytes();

    assert(myType() == Nda::Bytes);
    if (!mValue.uPtr)
        mValue.uPtr = new Nda::SharedBytes();
    return ((Nda::SharedBytes*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
const Nda::SharedBytes *NdaVariant::cInternalBytes() const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->cInternalBytes();

    assert(myType() == Nda::Bytes);
    return ((Nda::SharedBytes*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::detachBytes()
{
    assert(myType() == Nda::Bytes);

    if (internalBytes()->refCount() <= 1)
        return;

    auto *newBytes = new Nda::SharedBytes();
    newBytes->array() = internalBytes()->array();
    internalBytes()->releaseRef();
    mValue.uPtr = newBytes;
}

//-------------------------------------------------------------------------------------------------
Nda::SharedDict *NdaVariant::internalDict()
{
    if (myType() == Nda::Reference)
        return internalReference()->internalDict();

    assert(myType() == Nda::Dict);
    if (!mValue.uPtr)
        mValue.uPtr = new Nda::SharedDict();
    return ((Nda::SharedDict*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
const Nda::SharedDict   *NdaVariant::cInternalDict() const
{
    if (myType() == Nda::Reference)
        return cInternalReference()->cInternalDict();

    assert(myType() == Nda::Dict);
    return ((Nda::SharedDict*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::detachDict()
{
    assert(myType() == Nda::Dict);
    assert(mValue.uPtr);
    if (internalDict()->refCount() <= 1)
        return;

    auto *newDict = new Nda::SharedDict();
    newDict->dict() = internalDict()->dict(); // deep copy
    internalDict()->releaseRef();
    mValue.uPtr = newDict;
}

//-------------------------------------------------------------------------------------------------
Nda::SharedData *NdaVariant::internalSharedObject()
{
    switch (myType()) {
    case Nda::String:
    case Nda::List:
    case Nda::Bytes:
    case Nda::Dict:
        return (Nda::SharedData*)mValue.uPtr;
        break;
    default:
        assert(0 && "not implemented");
    }
    assert(0 && "never reach this code");
    return nullptr;
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
        if (mValue.uInt64 >= -((int64_t(1) << 53) - 1) && mValue.uInt64 <= ((int64_t(1) << 53) - 1)) {
            value = static_cast<double>(mValue.uInt64);
            return true;
        }
    } break;

    case Nda::Supernatural: {
        if (mValue.uInt64 >= 0 && mValue.uInt64 <= ((int64_t(1) << 53) - 1)) {
            value = static_cast<double>(mValue.uUInt64);
            return true;
        }
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

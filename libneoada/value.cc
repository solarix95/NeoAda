#include "value.h"
#include "sharedstring.h"
#include "numericparser.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <sstream>
#include <limits>

#define OP_SPACESHIP(v1, v2) ((int64_t)((v1) == (v2) ? 0 : ((v1) > (v2) ? +1 : -1)))

//-------------------------------------------------------------------------------------------------
NadaValue::NadaValue()
    : mType(Nada::Undefined)
{
    mValue.uInt64 = 0;
}

//-------------------------------------------------------------------------------------------------
NadaValue::NadaValue(const NadaValue &other)
    : mType(Nada::Undefined)
{
    mValue.uInt64 = 0;
    assignOther(other);
}

//-------------------------------------------------------------------------------------------------
NadaValue::~NadaValue()
{
    reset();
}

//-------------------------------------------------------------------------------------------------
void NadaValue::initAny()
{
    reset();
    mType = Nada::Any;
}

//-------------------------------------------------------------------------------------------------
void NadaValue::initType(Nada::Type t)
{
    reset();
    mType = t;

    switch (mType) {
    case Nada::Undefined: break;
    case Nada::Any:       break;
    case Nada::Number:       mValue.uDouble = 0.0; break;
    case Nada::Natural:      mValue.uInt64  =  0;  break;
    case Nada::Supernatural: mValue.uInt64  =  0;  break;
    case Nada::Boolean:      mValue.uByte   =  0;  break;
    case Nada::Byte:         mValue.uByte   =  0;  break;
    case Nada::Character:    mValue.uByte   =  0;  break;
    case Nada::String:       mValue.uPtr    =  nullptr; break;
    case Nada::Struct:       mValue.uPtr    =  nullptr; break;
    default:
        assert(0 && "not implemented");
    }
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::fromString(const std::string &value)
{
    reset();
    mType = Nada::String;
    if (!value.empty())
        mValue.uPtr = new NadaSharedString(value);
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::fromNumber(const std::string &value)
{
    reset();

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
        assert(mType == Nada::Undefined);
        return false;
    } catch (...) {
        // std::cerr << "Fehler: " << e.what() << std::endl;
        assert(mType == Nada::Undefined);
        return false;
    }

    assert(mType == Nada::Undefined);
    return false;
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::fromNumber(uint64_t value)
{
    reset();
    mValue.uUInt64 = value;
    mType = Nada::Supernatural;
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::fromNumber(int64_t value)
{
    reset();
    mValue.uInt64 = value;
    mType = Nada::Natural;
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::fromNumber(double value)
{
    reset();
    mValue.uDouble = value;
    mType = Nada::Number;
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::fromDoubleNan()
{
    return fromNumber(std::numeric_limits<double>::quiet_NaN());
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::fromBool(bool value)
{
    reset();
    mValue.uByte = value;
    mType = Nada::Boolean;
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::toBool(bool *ok) const
{
    if (ok) *ok = false;
    switch (mType) {
    case Nada::Undefined: return false; break;
    case Nada::Any:       return false; break;
    case Nada::Number:    return false; break;
    case Nada::Natural:
        if (ok) *ok = true;
        return mValue.uInt64 != 0;
        break;
    case Nada::Supernatural:
        if (ok) *ok = true;
        return mValue.uInt64 != 0;
        break;
    case Nada::Boolean:
        if (ok) *ok = true;
        return mValue.uByte != 0;
        break;
    case Nada::Byte:
        if (ok) *ok = true;
        return (bool)mValue.uByte;
        break;
    case Nada::Character: return false; break;
    case Nada::String:    return false; break;
    case Nada::Struct:    return false; break;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
int64_t NadaValue::toInt64(bool *ok) const
{
    if (ok) *ok = false;
    switch (mType) {
    case Nada::Undefined: return 0; break;
    case Nada::Any:       return 0; break;
    case Nada::Number: {
        int ret;
        bool isInt = exact32BitInt(ret);
        if (isInt) {
            if (ok) *ok = true;
            return ret;
        }
        return 0;
    }
    case Nada::Natural:
        if (ok) *ok = true;
        return mValue.uInt64;
        break;
    case Nada::Supernatural:
        if (mValue.uInt64 >= 0) {
            if (ok) *ok = true;
            return (int64_t)mValue.uUInt64;
        }
        break;
    case Nada::Boolean:
        if (ok) *ok = true;
        return mValue.uByte;
        break;
    case Nada::Byte:
        if (ok) *ok = true;
        return (bool)mValue.uByte;
        break;
    case Nada::Character: return false; break;
    case Nada::String:    return false; break;
    case Nada::Struct:    return false; break;
    }
    return false;

}

//-------------------------------------------------------------------------------------------------
bool NadaValue::isNan() const
{
    return (mType == Nada::Number) && std::isnan(mValue.uDouble);
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::assign(const NadaValue &other)
{
    if (this == &other)
        return true;

    switch (mType) {
    case Nada::Undefined: return false; break;
    case Nada::Any: {
        assignOther(other);
        return true;
    } break;
    case Nada::Number: {
        if (other.mType == mType) {
            mValue.uDouble = other.mValue.uDouble;
            return true;
        }
        if (other.type() == Nada::Natural) {
            mValue.uDouble = (double)other.mValue.uInt64;
            return true;
        }
    } break;
    case Nada::Natural: {
        if (other.mType == mType) {
            mValue.uInt64 = other.mValue.uInt64;
            return true;
        }
    } break;
    case Nada::Supernatural: {
        if (other.mType == mType) {
            mValue.uUInt64 = other.mValue.uUInt64;
            return true;
        }
    } break;
    case Nada::Boolean: {
        if (other.mType == mType) {
            mValue.uByte = other.mValue.uByte;
            return true;
        }
    } break;
    case Nada::Byte: {
        if (other.mType == mType) {
            mValue.uByte = other.mValue.uByte;
            return true;
        }
    } break;
    case Nada::String: {
        if (other.mType == mType) {
            if (other.mValue.uPtr == mValue.uPtr)
                return true;

            reset();
            assignOtherString(other);
            return true;
        }
    } break;

    case Nada::Character: return false; break;
    case Nada::Struct:    return false; break;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::equal(const NadaValue &other, bool *ok) const
{
    bool done;
    if (ok) *ok = false;

    auto res = spaceship(other, &done);

    if (!done)
        return false;

    if (res.isNan()) {
        if (ok) *ok = true;
        return false;
    }

    if (ok) *ok = true;
    return res.toInt64() == 0;
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::logicalAnd(const NadaValue &other, bool *ok) const
{
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
bool NadaValue::logicalOr(const NadaValue &other, bool *ok) const
{
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
bool NadaValue::logicalXor(const NadaValue &other, bool *ok) const
{
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
bool NadaValue::greaterThen(const NadaValue &other, bool *ok) const
{
    bool done;
    if (ok) *ok = false;

    auto res = spaceship(other, &done);

    if (!done)
        return false;

    if (res.isNan()) {
        if (ok) *ok = true;
        return false;
    }

    if (ok) *ok = true;
    return res.toInt64() > 0;
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::lessThen(const NadaValue &other, bool *ok) const
{
    bool done;
    if (ok) *ok = false;

    auto res = spaceship(other, &done);

    if (!done)
        return false;

    if (res.isNan()) {
        if (ok) *ok = true;
        return false;
    }

    if (ok) *ok = true;
    return res.toInt64() < 0;
}

//-------------------------------------------------------------------------------------------------
NadaValue NadaValue::spaceship(const NadaValue &other, bool *ok) const
{
    if (ok) *ok = false;

    NadaValue ret;

    switch (mType) {
    case Nada::Undefined: return NadaValue();
    case Nada::Any:       return NadaValue();
    case Nada::Number:
        if (std::isnan(mValue.uDouble) || (other.type() == Nada::Number && std::isnan(other.mValue.uDouble))) {
            if (ok) *ok = true;
            ret.fromDoubleNan();
            return ret;
        }
        if (other.mType != mType) {
            int v1, v2;
            bool v1IsInt = exact32BitInt(v1);
            bool v2IsInt = other.exact32BitInt(v2);
            if (v1IsInt && v2IsInt) {
                if (ok) *ok = true;
                ret.fromNumber(OP_SPACESHIP(v1,v2));
                return ret;
            }
            if (v2IsInt) {
                if (ok) *ok = true;
                ret.fromNumber(OP_SPACESHIP(mValue.uDouble,(double)v2));
                return ret;
            }
            double otherVal;
            if (other.exact64BitDbl(otherVal)) {
                if (ok) *ok = true;
                ret.fromNumber(OP_SPACESHIP(mValue.uDouble,otherVal));
                return ret;
            }
            return ret; // giving up...
        }
        if (ok) *ok = true;
        ret.fromNumber(OP_SPACESHIP(mValue.uDouble,other.mValue.uDouble));
        break;
    case Nada::Natural:
        if (other.mType != mType)
            return ret;
        if (ok) *ok = true;
        ret.fromNumber(OP_SPACESHIP(mValue.uInt64,other.mValue.uInt64));
        break;
    case Nada::Supernatural:
        if (other.mType != mType)
            return ret;
        if (ok) *ok = true;
        ret.fromNumber(OP_SPACESHIP(mValue.uUInt64,other.mValue.uUInt64));
        break;
    case Nada::Boolean:
    case Nada::Byte:
    case Nada::Character:
        if (other.mType != mType)
            return ret;
        if (ok) *ok = true;
        ret.fromNumber(OP_SPACESHIP(mValue.uByte,other.mValue.uByte));
        break;
    case Nada::String:
        if (other.mType != mType)
            return ret;
        if (ok) *ok = true;
        if (!mValue.uPtr && !other.mValue.uPtr)   // both are empty
            ret.fromNumber((int64_t)  0);
        else if (!mValue.uPtr)                    // I'm empty the other is not -> not greater
            ret.fromNumber((int64_t) -1);
        if (!other.mValue.uPtr)                //  I'm not empty the other is empty -> greater
            ret.fromNumber((int64_t) +1);
        else
            ret.fromNumber(OP_SPACESHIP(cInternalString()->cValue(),other.cInternalString()->cValue()));
        break;
    case Nada::Struct:
        assert(0 && "not implemented");
        return ret;
    }
    return ret;
}


//-------------------------------------------------------------------------------------------------
NadaValue NadaValue::concat(const NadaValue &other, bool *ok) const
{
    if (ok) *ok = false;
    if (mType != other.mType)
        return NadaValue();

    switch (mType) {
    case Nada::String: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromString(cInternalString()->cValue() + other.cInternalString()->cValue());
        return ret;
    } break;
    case Nada::Struct: {
        assert(0 && "not yet implemented");
        return NadaValue();
    } break;
    default:
        break;
    }

    return NadaValue();
}

//-------------------------------------------------------------------------------------------------
NadaValue NadaValue::subtract(const NadaValue &other, bool *ok) const
{
    if (ok) *ok = false;
    if (mType != other.mType)
        return NadaValue();

    switch (mType) {
    case Nada::Number: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uDouble - other.mValue.uDouble);
        return ret;
    } break;
    case Nada::Natural: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uInt64 - other.mValue.uInt64);
        return ret;
    } break;
    case Nada::Supernatural: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uUInt64 - other.mValue.uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NadaValue();
}

//-------------------------------------------------------------------------------------------------
NadaValue NadaValue::add(const NadaValue &other, bool *ok) const
{
    if (ok) *ok = false;
    if (mType != other.mType)
        return NadaValue();

    switch (mType) {
    case Nada::Number: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uDouble + other.mValue.uDouble);
        return ret;
    } break;
    case Nada::Natural: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uInt64 + other.mValue.uInt64);
        return ret;
    } break;
    case Nada::Supernatural: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uUInt64 + other.mValue.uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NadaValue();

}

//-------------------------------------------------------------------------------------------------
NadaValue NadaValue::modulo(const NadaValue &other, bool *ok) const
{
    if (ok) *ok = false;
    if (mType != other.mType)
        return NadaValue();

    switch (mType) {
    case Nada::Natural: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uInt64 % other.mValue.uInt64);
        return ret;
    } break;
    case Nada::Supernatural: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uUInt64 % other.mValue.uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NadaValue();
}

//-------------------------------------------------------------------------------------------------
NadaValue NadaValue::multiply(const NadaValue &other, bool *ok) const
{
    if (ok) *ok = false;
    if (mType != other.mType)
        return NadaValue();

    switch (mType) {
    case Nada::Number: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uDouble * other.mValue.uDouble);
        return ret;
    } break;
    case Nada::Natural: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uInt64 * other.mValue.uInt64);
        return ret;
    } break;
    case Nada::Supernatural: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uUInt64 * other.mValue.uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NadaValue();
}

//-------------------------------------------------------------------------------------------------
NadaValue NadaValue::division(const NadaValue &other, bool *ok) const
{
    if (ok) *ok = false;
    if (mType != other.mType)
        return NadaValue();

    switch (mType) {
    case Nada::Number: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uDouble / other.mValue.uDouble);
        return ret;
    } break;
    case Nada::Natural: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uInt64 / other.mValue.uInt64);
        return ret;
    } break;
    case Nada::Supernatural: {
        NadaValue ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uUInt64 / other.mValue.uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NadaValue();

}

//-------------------------------------------------------------------------------------------------
void NadaValue::unaryOperator(const std::string &op, bool *ok)
{
    if (ok) *ok = false;
    switch (mType) {
    case Nada::Undefined: return; break;
    case Nada::Any:       return; break;
    case Nada::Number: {
        if (op == "+") {
            if (ok) *ok = true;
            return;
        }
        if (op == "-") {
            // FIXME: nan? inf?
            if (ok) *ok = true;
            mValue.uDouble = -mValue.uDouble;
            return;
        }
    } break;
    case Nada::Natural:{
        if (op == "+") {
            if (ok) *ok = true;
            return;
        }
        if (op == "-") {
            if (ok) *ok = true;
            mValue.uInt64 = -mValue.uInt64;
            return;
        }
    } break;
    case Nada::Supernatural:{
        if (op == "+") {
            if (ok) *ok = true;
            return;
        }
    } break;
    case Nada::Boolean:
        break;
    case Nada::Byte:
        break;
    case Nada::Character:
        break;
    case Nada::String:
        break;
    case Nada::Struct:
        break;
    }
}

//-------------------------------------------------------------------------------------------------
void NadaValue::assignOther(const NadaValue &other)
{
    switch (other.mType) {
    case Nada::Undefined: return;
    case Nada::Any:       return;
    case Nada::Number:
    case Nada::Natural:
    case Nada::Supernatural:
    case Nada::Boolean:
    case Nada::Byte:
    case Nada::Character:
        mType = other.mType;
        mValue.uInt64 = other.mValue.uInt64;
        return;
    case Nada::String:
        assignOtherString(other);
        return;
    case Nada::Struct:
        assert(0 && "not implemented");
        return;
    }
}

//-------------------------------------------------------------------------------------------------
void NadaValue::assignOtherString(const NadaValue &other)
{
    if (mType == Nada::Undefined || mType == Nada::Any)
        mType = Nada::String;

    if (mType != Nada::String)
        return;

    if (mValue.uPtr) {
        internalString()->refCount();
        mValue.uPtr = nullptr;
    }

    if (!other.mValue.uPtr)
        return;

    mValue.uPtr = other.mValue.uPtr;
    internalString()->addRef();
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::setString(const std::string &newValue)
{
    if (mType != Nada::String)
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
            mValue.uPtr = new NadaSharedString(newValue);
            return true;
        }
        internalString()->value() = newValue;
    } else {
        mValue.uPtr = new NadaSharedString(newValue);
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
std::string NadaValue::toString() const
{
    std::ostringstream oss;

    switch (mType) {
    case Nada::Undefined: return "";
    case Nada::Any:       return "";
    case Nada::Number:
        oss << std::setprecision(15) << std::fixed << mValue.uDouble;
        break;
    case Nada::Natural:
        oss << mValue.uInt64;
        break;
    case Nada::Supernatural:
        oss << mValue.uUInt64;
        break;
    case Nada::Boolean:
    case Nada::Byte:
    case Nada::Character:
        return "NOT IMPLEMENTED";
    case Nada::String:
        if (mValue.uPtr)
            return cInternalString()->cValue();
        return "";
    case Nada::Struct:
        return "NOT IMPLEMENTED";
    }

    return oss.str();
}

//-------------------------------------------------------------------------------------------------
Nada::Type NadaValue::type() const
{
    return mType;
}

//-------------------------------------------------------------------------------------------------
NadaValue &NadaValue::operator=(const NadaValue &other)
{
    assignOther(other);
    return *this;
}

//-------------------------------------------------------------------------------------------------
int NadaValue::refCount() const
{
    if (!mValue.uPtr)
        return 0;

    if (mType == Nada::String)
        return cInternalString()->refCount();

    assert(0 && "Not Implemented");
    return 0;
}

//-------------------------------------------------------------------------------------------------
void NadaValue::reset()
{
    switch (mType) {
    case Nada::Undefined: return;
    case Nada::Any:
    case Nada::Number:
    case Nada::Natural:
    case Nada::Supernatural:
    case Nada::Boolean:
    case Nada::Byte:
    case Nada::Character:
        mType = Nada::Undefined;
        mValue.uInt64 = 0;
        return;
    case Nada::String:
        if (mValue.uPtr)
            ((NadaSharedString*)mValue.uPtr)->releaseRef();
        mType = Nada::Undefined;
        mValue.uInt64 = 0;
        return;
    case Nada::Struct:
        assert(0 && "not implemented");
        return;
    }

    assert(0 && "Don't reach code here");
}

//-------------------------------------------------------------------------------------------------
NadaSharedString *NadaValue::internalString()
{
    assert(mType == Nada::String);
    assert(mValue.uPtr);
    return ((NadaSharedString*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
const NadaSharedString *NadaValue::cInternalString() const
{
    assert(mType == Nada::String);
    assert(mValue.uPtr);

    return ((NadaSharedString*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::exact32BitInt(int &value) const
{
    if ((mType == Nada::Natural) && (mValue.uInt64 <= INT32_MAX) && (mValue.uInt64 >= INT32_MIN))
    {
        value = (int)mValue.uInt64;
        return true;
    }

    if ((mType == Nada::Supernatural) && (mValue.uUInt64 <= INT32_MAX))
    {
        value = (int)mValue.uUInt64;
        return true;
    }

    if ((mType == Nada::Number) && (std::floor(mValue.uDouble) == mValue.uDouble)
        && (mValue.uDouble >= INT32_MIN && mValue.uDouble <= INT32_MAX))
    {
        value = static_cast<int>(mValue.uDouble);
        return true;
    }

    if (mType == Nada::Byte) {
        value = mValue.uByte;
        return true;
    }

    if (mType == Nada::Character) {
        value = mValue.uChar;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
bool NadaValue::exact64BitDbl(double &value) const
{
    switch (mType) {
    case Nada::Undefined:
    case Nada::Any:
    case Nada::String:
    case Nada::Struct:
        return false;

    case Nada::Number: {
        value = mValue.uDouble;
        return true;
    } break;
    case Nada::Natural: {
        if (mValue.uInt64 >= -((int64_t(1) << 53) - 1) && mValue.uInt64 <= ((int64_t(1) << 53) - 1))
            value = static_cast<double>(mValue.uInt64);
        return true;
    } break;

    case Nada::Supernatural: {
        if (mValue.uInt64 >= 0 && mValue.uInt64 <= ((int64_t(1) << 53) - 1))
            value = static_cast<double>(mValue.uUInt64);
        return true;
    } break;
    case Nada::Boolean:
    case Nada::Byte:
        value = static_cast<double>(mValue.uByte);
        return true;
    case Nada::Character:
        value = static_cast<double>(mValue.uChar);
        return true;
    }

    return false;
}

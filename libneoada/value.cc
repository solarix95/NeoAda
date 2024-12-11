#include "value.h"
#include "sharedstring.h"
#include "numericparser.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <sstream>
#include <limits>
#include <regex>

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
bool NadaValue::greaterThen(const NadaValue &other, bool *ok) const
{
    if (ok) *ok = false;

    switch (other.mType) {
    case Nada::Undefined: return false;
    case Nada::Any:       return false;
    case Nada::Number:
        if (other.mType != mType)
            return false;
        if (ok) *ok = true;
        return mValue.uDouble > other.mValue.uDouble;
        break;
    case Nada::Natural:
        if (other.mType != mType)
            return false;
        if (ok) *ok = true;
        return mValue.uInt64 > other.mValue.uInt64;
        break;
    case Nada::Supernatural:
        if (other.mType != mType)
            return false;
        if (ok) *ok = true;
        return mValue.uUInt64 > other.mValue.uUInt64;
        break;
    case Nada::Boolean:
    case Nada::Byte:
    case Nada::Character:
        if (other.mType != mType)
            return false;
        if (ok) *ok = true;
        return mValue.uByte > other.mValue.uByte;
    case Nada::String:
        if (other.mType != mType)
            return false;
        if (ok) *ok = true;
        if (!mValue.uPtr && !other.mValue.uPtr) // both are empty
            return false;
        if (!mValue.uPtr)                      // I'm empty the other is not -> not greater
            return false;
        if (!other.mValue.uPtr)                //  I'm not empty the other is empty -> greater
            return true;
        return (cInternalString()->cValue() > other.cInternalString()->cValue());
    case Nada::Struct:
        assert(0 && "not implemented");
        return false;
    }
    return false;

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

#include "value.h"
#include "sharedstring.h"
#include <cassert>

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
void NadaValue::fromString(const std::string &value)
{
    reset();
    mType = Nada::String;
    if (!value.empty())
        mValue.uPtr = new NadaSharedString(value);
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
    switch (mType) {
    case Nada::Undefined: return "";
    case Nada::Any:
    case Nada::Number:
    case Nada::Natural:
    case Nada::Supernatural:
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

    assert(0 && "Don't reach code here");
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

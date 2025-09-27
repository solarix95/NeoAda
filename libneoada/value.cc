#include "value.h"

//-------------------------------------------------------------------------------------------------
NdaValue::NdaValue()
    : mType(Nda::Undefined)

{}

//-------------------------------------------------------------------------------------------------
NdaValue::NdaValue(const char *v)
    : mType(Nda::String)
    , mSValue(v)
{
}

//-------------------------------------------------------------------------------------------------
NdaValue::NdaValue(const std::string &v)
    : mType(Nda::String)
    , mSValue(v)
{
}

//-------------------------------------------------------------------------------------------------
NdaValue::NdaValue(double v)
    : mType(Nda::Number)
{
    mUValue.dValue = v;
}

//-------------------------------------------------------------------------------------------------
NdaValue::NdaValue(int64_t v)
    : mType(Nda::Natural)
{
    mUValue.iValue = v;

}

//-------------------------------------------------------------------------------------------------
NdaValue::NdaValue(bool v)
    : mType(Nda::Boolean)
{
    mUValue.bValue = v;

}

//-------------------------------------------------------------------------------------------------
NdaValue::NdaValue(const std::vector<NdaValue> &v)
    : mType(Nda::List)
    , mLValue(v)
{
}

//-------------------------------------------------------------------------------------------------
Nda::Type NdaValue::type() const
{
    return mType;
}

//-------------------------------------------------------------------------------------------------
bool NdaValue::isValid() const
{
    return mType != Nda::Undefined;
}

//-------------------------------------------------------------------------------------------------
std::string NdaValue::toString() const
{
    switch(mType) {
    case Nda::String: return mSValue; break;
    default: break;
    }
    return "";
}

//-------------------------------------------------------------------------------------------------
bool NdaValue::toBool() const
{
    switch(mType) {
    case Nda::Boolean: return mUValue.bValue; break;
    default: break;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
double NdaValue::toDouble() const
{
    switch(mType) {
    case Nda::Number: return mUValue.dValue; break;
    default: break;
    }
    return 0.0;
}

//-------------------------------------------------------------------------------------------------
int64_t NdaValue::toInt64() const
{
    switch(mType) {
    case Nda::Natural: return mUValue.iValue; break;
    default: break;
    }
    return 0;
}

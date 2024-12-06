#include "value.h"

//-------------------------------------------------------------------------------------------------
NadaValue::NadaValue(Nada::Type type)
    : mType(type)
{
    mValue.uInt64 = 0;
}

//-------------------------------------------------------------------------------------------------
std::string NadaValue::toString() const
{
    return "Gugusli";
}

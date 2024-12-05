#include "value.h"

//-------------------------------------------------------------------------------------------------
NadaValue::NadaValue(Nada::Type type)
    : mType(type)
{
    mValue.uInt64 = 0;
}

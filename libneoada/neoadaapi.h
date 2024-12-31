#ifndef NEOADAAPI_H
#define NEOADAAPI_H

#include "variant.h"

class NdaException;
namespace NeoAda
{
using Exception = NdaException;

NdaVariant evaluate(const std::string &shortScript, Exception *exception = nullptr);
}


#endif // NEOADAAPI_H

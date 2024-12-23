#ifndef NEOADAAPI_H
#define NEOADAAPI_H

#include "value.h"

class NadaException;
namespace NeoAda
{
using Exception = NadaException;

NadaValue evaluate(const std::string &shortScript, Exception *exception = nullptr);
}


#endif // NEOADAAPI_H

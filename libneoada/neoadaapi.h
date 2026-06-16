#ifndef NEOADAAPI_H
#define NEOADAAPI_H

#include "state.h"
#include "variant.h"
#include "formula.h"

class NdaException;
namespace NeoAda
{
using Exception = NdaException;

NdaVariant evaluate(const std::string &shortScript, NdaState &state, Exception *exception = nullptr);
NdaVariant evaluateFormula(const std::string &formula, NdaState &state, Exception *exception = nullptr);
NdaFormula prepareFormula(const std::string &formula, NdaState &state, Exception *exception = nullptr);
NdaVariant executeFormula(NdaFormula &formula, NdaState &state, Exception *exception = nullptr);
}


#endif // NEOADAAPI_H

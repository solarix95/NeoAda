#ifndef LIB_NEOADA_RUNNABLE_H
#define LIB_NEOADA_RUNNABLE_H

#include "utils.h"

class NdaVariant;
class NdaInterpreter;

namespace Nda {

enum CallMetaType {
    CallNOP,
    CallType,
    ConditionalCall, // elseIf
    FallbackCall,    // else
    NcIdentifier,
    NcStringLiteral,
    NcNumberLiteral,
    NcBoolLiteral,
    NcListLiteral,
    NcMethodContext,
};

struct Runnable
{
    void (NdaInterpreter::*call)(Runnable* self);
    CallMetaType      type;

    Nda::LowerString  value;
    Runnable         *parent;
    Runnable        **children;
    int               childrenCount;

    int               line;
    int               column;

    NdaVariant       *variantCache;

    Runnable(int l, int c, int ccount, const std::string& v = "");
    Runnable(int l, int c, int ccount, const Nda::LowerString& v);
    ~Runnable();
};

}

#endif // RUNNABLE_H

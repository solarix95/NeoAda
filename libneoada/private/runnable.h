#ifndef LIB_NEOADA_RUNNABLE_H
#define LIB_NEOADA_RUNNABLE_H

#include "utils.h"
#include <cstdint>

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
    NcDictLiteral,
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

    uint32_t          sourceId;
    int               line;
    int               column;

    NdaVariant       *variantCache;
    int               symbolIndex;
    int               symbolScope;
    bool              symbolIsGlobal;

    Runnable(int l, int c, int ccount, const std::string& v = "");
    Runnable(int l, int c, int ccount, const Nda::LowerString& v);
    ~Runnable();
};

}

#endif // RUNNABLE_H

#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include "type.h"

class NadaValue
{
public:
    NadaValue(Nada::Type type = Nada::Undefined);

    void initDefaultValue();

private:
    Nada::Type   mType;

    union {
        char          uChar;
        unsigned char uByte;
        double        uDouble;
        int64_t       uInt64;
        uint64_t      uUInt64;
        void         *uPtr;
    }            mValue;
};

#endif // VALUE_H

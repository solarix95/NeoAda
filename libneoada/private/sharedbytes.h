#ifndef LIB_NEOADA_SHAREDBYTES_H
#define LIB_NEOADA_SHAREDBYTES_H

#include <vector>

#include "../variant.h"
#include "shareddata.h"

namespace Nda {

class SharedBytes : public Nda::SharedData
{
public:
    SharedBytes();

    inline std::vector<NdaVariant>        &array()        { return mArray; }
    inline const std::vector<NdaVariant>  &cArray() const { return mArray; }

private:
    std::vector<NdaVariant>  mArray;
};

}

#endif // LIB_NEOADA_SHAREDBYTES_H

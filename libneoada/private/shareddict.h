#ifndef LIB_NEOADA_SHAREDDICT_H
#define LIB_NEOADA_SHAREDDICT_H

#include <unordered_map>
#include <map>

#include "../variant.h"
#include "shareddata.h"

namespace Nda {

using StdMap = std::map<NdaVariant,NdaVariant>;

class SharedDict : public Nda::SharedData
{
public:
    SharedDict();

    inline StdMap        &dict()        { return mDict; }
    inline const StdMap  &cDict() const { return mDict; }

private:
    StdMap  mDict;
};

}

#endif // LIB_NEOADA_SHAREDDICT_H

#ifndef LIB_NEOADA_SHAREDDICT_H
#define LIB_NEOADA_SHAREDDICT_H

#include <unordered_map>
#include <map>

#include "../variant.h"
#include "shareddata.h"

namespace Nda {

struct DictKeyLess
{
    bool operator()(const NdaVariant &left, const NdaVariant &right) const
    {
        return left.compareDictKey(right) < 0;
    }
};

using StdMap = std::map<NdaVariant,NdaVariant,DictKeyLess>;

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

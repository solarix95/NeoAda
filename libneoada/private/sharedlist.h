#ifndef SHAREDLIST_H
#define SHAREDLIST_H

#include <vector>

#include "../variant.h"
#include "shareddata.h"

namespace Nda {


class SharedList : public Nda::SharedData
{
public:
    SharedList();

    inline std::vector<NdaVariant>        &array()        { return mArray; }
    inline const std::vector<NdaVariant>  &cArray() const { return mArray; }

private:
    std::vector<NdaVariant>  mArray;

};

}

#endif // SHAREDLIST_H

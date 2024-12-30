#ifndef SHAREDLIST_H
#define SHAREDLIST_H

#include <vector>

#include "../value.h"
#include "shareddata.h"

namespace Nda {


class SharedList : public Nda::SharedData
{
public:
    SharedList();

    inline std::vector<NadaValue>        &array()        { return mArray; }
    inline const std::vector<NadaValue>  &cArray() const { return mArray; }

private:
    std::vector<NadaValue>  mArray;

};

}

#endif // SHAREDLIST_H

#include "shareddata.h"
#include <cassert>

namespace Nda {

//-------------------------------------------------------------------------------------------------
SharedData::SharedData() : mReferences(1) {}

SharedData::~SharedData()
{
}

//-------------------------------------------------------------------------------------------------
void SharedData::addRef()
{
    ++mReferences;
}

//-------------------------------------------------------------------------------------------------
void SharedData::releaseRef()
{
    assert(mReferences > 0);

    if (--mReferences == 0)
        delete this;
}

}

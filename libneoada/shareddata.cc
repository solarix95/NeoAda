#include "shareddata.h"
#include <cassert>

//-------------------------------------------------------------------------------------------------
NadaSharedData::NadaSharedData() : mReferences(1) {}

NadaSharedData::~NadaSharedData()
{
}

//-------------------------------------------------------------------------------------------------
void NadaSharedData::addRef()
{
    ++mReferences;
}

//-------------------------------------------------------------------------------------------------
void NadaSharedData::releaseRef()
{
    assert(mReferences > 0);

    if (--mReferences == 0)
        delete this;
}

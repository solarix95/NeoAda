#include "runnable.h"
#include "../variant.h" // delete variantCache

//-------------------------------------------------------------------------------------------------
Nda::Runnable::Runnable(int l, int c, int ccount, const std::string &v)
    : value(v), parent(nullptr), line(l)
    , column(c), variantCache(nullptr)
    , symbolIndex(-1), symbolScope(-1), symbolIsGlobal(false)
{
    childrenCount = ccount;
    if (childrenCount > 0) {
        children = new Runnable*[childrenCount];
        // children-pointer will be set from caller
    }
}

//-------------------------------------------------------------------------------------------------
Nda::Runnable::Runnable(int l, int c, int ccount, const LowerString &v)
    : value(v), parent(nullptr), line(l)
    , column(c), variantCache(nullptr)
    , symbolIndex(-1), symbolScope(-1), symbolIsGlobal(false)
{
    childrenCount = ccount;
    if (childrenCount > 0) {
        children = new Runnable*[childrenCount];
        // children-pointer will be set from caller
    }
}

//-------------------------------------------------------------------------------------------------
Nda::Runnable::~Runnable()
{
    if (childrenCount > 0) {
        for (int i=0; i<childrenCount; i++)
            delete children[i];
        delete [] children;
    }

    if (variantCache)
        delete variantCache;
}

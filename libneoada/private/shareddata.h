#ifndef LIB_NEOADA_SHAREDDATA_H
#define LIB_NEOADA_SHAREDDATA_H

/*
    NadaSharedData

    To be used only on HEAP. Shared-Data-Container for Copy-On-Write Values
    like "String" and "Struct"
*/

namespace Nda {

class SharedData
{
public:
    SharedData();
    virtual ~SharedData();

    void addRef();
    void releaseRef();

    inline int  refCount() const { return mReferences; }

    SharedData(const SharedData&) = delete;            // No Copy
    SharedData& operator=(const SharedData&) = delete; // No Assigment

private:
    int mReferences;
};

}

#endif // SHAREDDATA_H

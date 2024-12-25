#ifndef SHAREDDATA_H
#define SHAREDDATA_H

/*
    NadaSharedData

    To be used only on HEAP. Shared-Data-Container for Copy-On-Write Values
    like "String" and "Struct"
*/

class NadaSharedData
{
public:
    NadaSharedData();
    virtual ~NadaSharedData();

    void addRef();
    void releaseRef();

    inline int  refCount() const { return mReferences; }

    NadaSharedData(const NadaSharedData&) = delete;            // No Copy
    NadaSharedData& operator=(const NadaSharedData&) = delete; // No Assigment

private:
    int mReferences;
};

#endif // SHAREDDATA_H

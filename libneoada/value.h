#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <vector>
#include "private/type.h"

class NdaValue
{
public:
    NdaValue();
    NdaValue(const char *v);
    NdaValue(const std::string &v);
    NdaValue(double   v);
    NdaValue(int64_t  v);
    NdaValue(bool     v);
    NdaValue(const std::vector<NdaValue> &v);

    Nda::Type   type()     const;
    bool        isValid()  const; // type != undefined

    std::string toString() const;
    bool        toBool()   const;
    double      toDouble() const;
    int64_t     toInt64()  const;

private:
    Nda::Type             mType;

    union {
        bool     bValue;
        double   dValue;
        int64_t  iValue;
    }                     mUValue;
    std::string           mSValue;
    std::vector<NdaValue> mLValue;
};


#endif // VALUE_H

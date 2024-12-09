#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include <vector>
#include <string>
#include "type.h"

class NadaSharedString;
class NadaValue
{
public:
    NadaValue();
    NadaValue(const NadaValue &other);
    ~NadaValue();

    bool fromString(const std::string &value);
    bool fromNumber(const std::string &value);
    bool fromNumber(uint64_t value);
    bool fromNumber(int64_t value);
    bool fromNumber(double value);
    bool fromBool(bool value);

    bool toBool(bool *ok) const;

    void initDefaultValue();

    // NeoAda-Operators
    bool greaterThen(const NadaValue &other, bool *ok) const;

    const std::string &cStringValue() const;

    bool        setString(const std::string &newValue);
    std::string toString() const;
    Nada::Type  type() const;

    NadaValue& operator=(const NadaValue&other);

    // Unit-Test only
    int         refCount() const;


private:
    void reset();
    void assignOther(const NadaValue &other);
    void assignOtherString(const NadaValue &other);

    NadaSharedString *internalString();
    const NadaSharedString *cInternalString() const;

    Nada::Type   mType;

    union {
        char          uChar;
        unsigned char uByte;
        double        uDouble;
        int64_t       uInt64;
        uint64_t      uUInt64;
        void         *uPtr;
    }            mValue;
};

using NadaValues = std::vector<NadaValue>;


#endif // VALUE_H

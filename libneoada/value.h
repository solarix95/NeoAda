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

    void initAny();
    void initType(Nada::Type t);
    bool fromString(const std::string &value);
    bool fromNumber(const std::string &value);
    bool fromNumber(uint64_t value);
    bool fromNumber(int64_t value);
    bool fromNumber(double value);
    bool fromBool(bool value);

    bool toBool(bool *ok) const;



    // NeoAda-Operators
    bool assign(const NadaValue &other);
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

    // Helper unterschiedlicher Datentypen
    NadaSharedString *internalString();
    const NadaSharedString *cInternalString() const;
    bool exact32BitInt(int &value) const;
    bool exact64BitDbl(double &value) const;


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

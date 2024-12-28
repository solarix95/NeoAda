#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include <vector>
#include <string>
#include "private/type.h"

class NadaSharedString;
class NadaValue
{
public:
    NadaValue();
    NadaValue(const NadaValue &other);
    ~NadaValue();

    void reset();
    void initAny();
    void initType(Nada::Type t);
    bool fromString(const std::string &value);
    bool fromNumber(const std::string &value);
    bool fromNumber(uint64_t value);
    bool fromNumber(int64_t value);
    bool fromNumber(double value);
    bool fromDoubleNan();
    bool fromBool(bool value);
    void fromReference(NadaValue *other);

    bool    toBool(bool *ok = nullptr) const;
    int64_t toInt64(bool *ok = nullptr) const;
    bool    isNan() const;


    // NeoAda-Operators
    bool      assign(const NadaValue &other);
    bool      equal(const NadaValue &other, bool *ok = nullptr) const;
    bool      logicalAnd(const NadaValue &other, bool *ok = nullptr) const;
    bool      logicalOr(const NadaValue &other, bool *ok = nullptr) const;
    bool      logicalXor(const NadaValue &other, bool *ok = nullptr) const;
    bool      greaterThen(const NadaValue &other, bool *ok = nullptr) const;
    bool      lessThen(const NadaValue &other, bool *ok = nullptr) const;

    NadaValue spaceship(const NadaValue &other, bool *ok = nullptr) const;

    NadaValue concat(const NadaValue &other, bool *ok = nullptr) const;
    NadaValue subtract(const NadaValue &other, bool *ok= nullptr) const;
    NadaValue add(const NadaValue &other, bool *ok= nullptr) const;
    NadaValue modulo(const NadaValue &other, bool *ok= nullptr) const;
    NadaValue multiply(const NadaValue &other, bool *ok= nullptr) const;
    NadaValue division(const NadaValue &other, bool *ok= nullptr) const;

    void unaryOperator(const std::string &op, bool *ok = nullptr);


    const std::string &cStringValue() const;

    bool        setString(const std::string &newValue);
    std::string toString() const;
    Nada::Type  type() const;

    NadaValue& operator=(const NadaValue&other);

    // Unit-Test only
    int         refCount() const;

    static bool fromNumber(const std::string &value, int64_t &ret);

private:

    void assignOther(const NadaValue &other);
    void assignOtherString(const NadaValue &other);

    // Helper unterschiedlicher Datentypen
    NadaSharedString       *internalString();
    const NadaSharedString *cInternalString() const;
    NadaValue              *internalReference();
    const NadaValue        *cInternalReference() const;

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

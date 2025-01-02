#ifndef VARIANT_H
#define VARIANT_H

#include <stdint.h>
#include <vector>
#include <string>
#include "private/type.h"

namespace Nda {
class  SharedString;
class  SharedList;
}

class NdaVariant
{
public:
    NdaVariant(const Nda::RuntimeType *type = nullptr);
    NdaVariant(const NdaVariant &other);
    ~NdaVariant();

    void reset();
    void initType(const Nda::RuntimeType *type);
    void fromString(const Nda::RuntimeType *type, const std::string &value);

    bool fromNumberLiteral(const Nda::RuntimeType  *type,const std::string &value);
    void fromNumber(const Nda::RuntimeType *type, double value);
    bool fromNaturalLiteral(const Nda::RuntimeType *type,const std::string &value);
    void fromNatural(const Nda::RuntimeType *type, int64_t value);
    bool fromSNaturalLiteral(const Nda::RuntimeType *type,const std::string &value);
    void fromSNatural(const Nda::RuntimeType *type, uint64_t value);
    bool fromByteLiteral(const Nda::RuntimeType *type,const std::string &value);

    void  fromDoubleNan(const Nda::RuntimeType *type);
    void  fromBool(const Nda::RuntimeType *t, bool value);
    void  fromReference(const Nda::RuntimeType *type, NdaVariant *other);

    bool    toBool(bool *ok = nullptr) const;
    int64_t toInt64(bool *ok = nullptr) const;
    bool    isNan() const;


    // NeoAda-Operators
    bool      assign(const NdaVariant &other);
    bool      equal(const NdaVariant &other, bool *ok = nullptr) const;
    bool      logicalAnd(const NdaVariant &other, bool *ok = nullptr) const;
    bool      logicalOr(const NdaVariant &other, bool *ok = nullptr) const;
    bool      logicalXor(const NdaVariant &other, bool *ok = nullptr) const;
    bool      greaterThen(const NdaVariant &other, bool *ok = nullptr) const;
    bool      lessThen(const NdaVariant &other, bool *ok = nullptr) const;

    double     spaceship(const NdaVariant &other, bool *ok = nullptr) const;

    NdaVariant concat(const NdaVariant &other, bool *ok = nullptr) const;
    NdaVariant subtract(const NdaVariant &other, bool *ok= nullptr) const;
    NdaVariant add(const NdaVariant &other, bool *ok= nullptr) const;
    NdaVariant modulo(const NdaVariant &other, bool *ok= nullptr) const;
    NdaVariant multiply(const NdaVariant &other, bool *ok= nullptr) const;
    NdaVariant division(const NdaVariant &other, bool *ok= nullptr) const;

    NdaVariant unaryOperator(const std::string &op, bool *ok = nullptr) const;
    int        lengthOperator() const;

    // List interface
    int               listSize() const;
    void              appendToList(const NdaVariant &value);
    void              insertIntoList(int index, const NdaVariant &value);
    void              takeFromList(int index);
    NdaVariant&       writeAccess(int index);
    const NdaVariant& readAccess(int index) const;
    int               indexInList(const NdaVariant &value) const;
    bool              containsInList(const NdaVariant &value) const;
    void              reverseList();

    // String interface
    const std::string &cStringValue() const;

    bool        setString(const std::string &newValue);
    std::string toString() const;
    Nda::Type   type() const;
    Nda::Type   myType() const;
    const Nda::RuntimeType *runtimeType() const;

    NdaVariant& operator=(const NdaVariant&other);

    // Unit-Test only
    int         refCount() const;

    static Nda::Type numericType(const std::string &literal);
    static bool fromNumber(const std::string &value, int64_t &ret);

private:
    void assignOther(const NdaVariant &other);
    void assignOtherString(const NdaVariant &other);
    void assignOtherList(const NdaVariant &other);

    // Helper unterschiedlicher Datentypen
    Nda::SharedString       *internalString();
    const Nda::SharedString *cInternalString() const;
    NdaVariant               *internalReference();
    const NdaVariant         *cInternalReference() const;

    Nda::SharedList         *internalList();
    const Nda::SharedList   *cInternalList() const;
    void                     detachList();


    bool exact32BitInt(int &value) const;
    bool exact64BitDbl(double &value) const;

    const Nda::RuntimeType *mRuntimeType;

    union UValue {
        unsigned char uByte;
        double        uDouble;
        int64_t       uInt64;
        uint64_t      uUInt64;
        void         *uPtr;
    };

    UValue mValue;

    inline UValue       *uValue()        { return (myType() == Nda::Reference) ?  internalReference()->uValue()  : &mValue; }
    inline const UValue *cuValue() const { return (myType() == Nda::Reference) ? cInternalReference()->cuValue() : &mValue; }

};

using NadaValues = std::vector<NdaVariant>;


#endif // VARIANT_H

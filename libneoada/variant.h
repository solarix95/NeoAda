#ifndef LIB_NEOADA_VARIANT_H
#define LIB_NEOADA_VARIANT_H

#include <stdint.h>
#include <vector>
#include <string>
#include <utility>
#include "private/type.h"

namespace Nda {
class  SharedData;
class  SharedString;
class  SharedList;
class  SharedBytes;
class  SharedDict;
}

class NdaVariant
{
public:
    NdaVariant(const Nda::RuntimeType *type = nullptr);
    NdaVariant(const NdaVariant &other);
    ~NdaVariant();

    bool operator<(const NdaVariant &other) const; // std::map

    void reset();
    void initType(const Nda::RuntimeType *type);
    void fromString(const Nda::RuntimeType *type, const std::string &value);

    bool fromNumberLiteral(const Nda::RuntimeType  *type,const std::string &value);
    void fromNumber(const Nda::RuntimeType *type, double value);
    bool setNumber(double value);

    bool fromNaturalLiteral(const Nda::RuntimeType *type,const std::string &value);
    void fromNatural(const Nda::RuntimeType *type, int64_t value);
    bool setNatural(int64_t value);

    bool fromSNaturalLiteral(const Nda::RuntimeType *type,const std::string &value);
    void fromSNatural(const Nda::RuntimeType *type, uint64_t value);
    bool setSupernatural(uint64_t value);

    bool fromByteLiteral(const Nda::RuntimeType *type,const std::string &value);
    void fromByte(const Nda::RuntimeType *type, unsigned char value);
    bool setByte(unsigned char value);

    void  fromDoubleNan(const Nda::RuntimeType *type);
    void  fromBool(const Nda::RuntimeType *t, bool value);
    bool  setBool(bool value);

    void  fromReference(const Nda::RuntimeType *type, NdaVariant *other);

    bool    toBool(bool *ok = nullptr) const;
    double   toDouble(bool *ok = nullptr) const;
    int64_t  toInt64(bool *ok = nullptr) const;
    uint64_t toUInt64(bool *ok = nullptr) const;
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
    NdaVariant division(const NdaVariant &other, bool &dbz, bool *ok= nullptr) const;

    NdaVariant unaryOperator(const std::string &op, bool *ok = nullptr) const;
    int        lengthOperator() const;

    // List interface
    inline int        listSize() const { return lengthOperator(); }
    void              appendToList(const NdaVariant &value);
    void              insertIntoList(int index, const NdaVariant &value);
    void              takeFromList(int index);
    NdaVariant&       writeListAccess(int index);
    const NdaVariant& readAccess(int index) const;
    int               indexInList(const NdaVariant &value) const;
    bool              containsInList(const NdaVariant &value) const;
    void              reverseList();
    void              clearList();

    // Bytes interface
    inline int        bytesSize() const { return lengthOperator(); }
    bool              appendToBytes(const NdaVariant &value);
    NdaVariant&       writeBytesAccess(int index);
    const NdaVariant& readBytesAccess(int index) const;
    void              clearBytes();

    // Dict interface
    inline int        dictSize() const { return lengthOperator(); }
    void              appendToDict(const NdaVariant &key, const NdaVariant &value);
    bool              contains(const NdaVariant&) const;
    NdaVariant&       writeDictAccess(const NdaVariant &key);
    std::vector<std::pair<NdaVariant, NdaVariant>> dictItems() const;
    void              takeFromDict(const NdaVariant&);

    // generic string interface
    bool        setString(const std::string &newValue);
    std::string toString() const;
    Nda::Type   type() const;

    inline Nda::Type   myType() const { return mRuntimeType ? mRuntimeType->dataType : Nda::Undefined; }

    const Nda::RuntimeType *runtimeType() const;

    NdaVariant& operator=(const NdaVariant&other);

    void dereference();

    // Unit-Test only
    int         refCount() const;

    static Nda::Type numericType(const std::string &literal);
    static bool fromNumber(const std::string &value, int64_t &ret);

private:
    void assignOther(const NdaVariant &other);     // C++ Operator
    void assignAny(const NdaVariant &other);       // NeoAdas Any := ...
    void assignOtherString(const NdaVariant &other);
    void assignOtherList(const NdaVariant &other);
    void assignOtherBytes(const NdaVariant &other);
    void assignOtherDict(const NdaVariant &other);
    NdaVariant doubleAddition(const NdaVariant &other, bool *ok= nullptr) const;
    NdaVariant doubleSubtraction(const NdaVariant &other, bool *ok= nullptr) const;
    NdaVariant doubleDivision(const NdaVariant &other, bool &dbz, bool *ok= nullptr) const;
    NdaVariant doubleMultiply(const NdaVariant &other, bool *ok= nullptr) const;

    // Helper unterschiedlicher Datentypen
    Nda::SharedString       *internalString();
    const Nda::SharedString *cInternalString() const;
    NdaVariant               *internalReference();
    const NdaVariant         *cInternalReference() const;

    Nda::SharedList         *internalList();
    const Nda::SharedList   *cInternalList() const;
    void                     detachList();

    Nda::SharedBytes        *internalBytes();
    const Nda::SharedBytes  *cInternalBytes() const;
    void                     detachBytes();

    Nda::SharedDict         *internalDict();
    const Nda::SharedDict   *cInternalDict() const;
    void                     detachDict();

    Nda::SharedData         *internalSharedObject();

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

using NdaVariants = std::vector<NdaVariant>;


#endif // VARIANT_H

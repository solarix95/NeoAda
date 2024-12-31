#include "variant.h"
#include "private/sharedstring.h"
#include "private/numericparser.h"
#include "private/sharedlist.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <sstream>
#include <limits>
#include <algorithm> // std::find

#define OP_SPACESHIP(v1, v2) ((int64_t)((v1) == (v2) ? 0 : ((v1) > (v2) ? +1 : -1)))

bool       operator==(const NdaVariant &v1, const NdaVariant &v2);

//-------------------------------------------------------------------------------------------------
NdaVariant::NdaVariant()
    : mType(Nda::Undefined)
{
    mValue.uInt64 = 0;
}

//-------------------------------------------------------------------------------------------------
NdaVariant::NdaVariant(const NdaVariant &other)
    : mType(Nda::Undefined)
{
    mValue.uInt64 = 0;
    assignOther(other);
}

//-------------------------------------------------------------------------------------------------
NdaVariant::~NdaVariant()
{
    reset();
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::initAny()
{
    reset();
    mType = Nda::Any;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::initType(Nda::Type t)
{
    reset();
    mType = t;

    switch (mType) {
    case Nda::Undefined: break;
    case Nda::Any:       break;
    case Nda::Number:       mValue.uDouble = 0.0; break;
    case Nda::Natural:      mValue.uInt64  =  0;  break;
    case Nda::Supernatural: mValue.uInt64  =  0;  break;
    case Nda::Boolean:      mValue.uByte   =  0;  break;
    case Nda::Byte:         mValue.uByte   =  0;  break;
    case Nda::Character:    mValue.uByte   =  0;  break;

    case Nda::String:       mValue.uPtr    =  nullptr; break;
    case Nda::List:         mValue.uPtr    =  nullptr; break;
    case Nda::Dict:         mValue.uPtr    =  nullptr; break;
    case Nda::Struct:       mValue.uPtr    =  nullptr; break;
    default:
        assert(0 && "not implemented");
    }
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromString(const std::string &value)
{
    reset();
    mType = Nda::String;
    if (!value.empty())
        mValue.uPtr = new Nda::SharedString(value);
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromNumber(const std::string &value)
{
    reset();

    std::string cleanLiteral = NadaNumericParser::removeSeparators(value);

    try {
        if (NadaNumericParser::isBasedLiteral(cleanLiteral)) {
            bool ok;
            uint64_t value = NadaNumericParser::parseBasedLiteral(cleanLiteral, ok);
            if (!ok)
                return false;
            return fromNumber(value);
        }

        if (NadaNumericParser::isFloatingPointLiteral(cleanLiteral)) {
            double value = std::stod(cleanLiteral);
            return fromNumber(value);
        }

        uint64_t uintValue = std::stoull(cleanLiteral, nullptr, 0);
        if (uintValue <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            return fromNumber(static_cast<int64_t>(uintValue));
        } else {
            return fromNumber(uintValue);
        }
    } catch (const std::runtime_error&) {
        // std::cerr << "Fehler: " << e.what() << std::endl;
        assert(mType == Nda::Undefined);
        return false;
    } catch (...) {
        // std::cerr << "Fehler: " << e.what() << std::endl;
        assert(mType == Nda::Undefined);
        return false;
    }

    assert(mType == Nda::Undefined);
    return false;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromNumber(uint64_t value)
{
    reset();
    mValue.uUInt64 = value;
    mType = Nda::Supernatural;
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromNumber(int64_t value)
{
    reset();
    mValue.uInt64 = value;
    mType = Nda::Natural;
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromNumber(double value)
{
    reset();
    mValue.uDouble = value;
    mType = Nda::Number;
    return true;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromDoubleNan()
{
    return fromNumber(std::numeric_limits<double>::quiet_NaN());
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromBool(bool value)
{
    reset();
    mValue.uByte = value;
    mType = Nda::Boolean;
    return true;
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::fromReference(NdaVariant *other)
{
    assert(other);
    reset();
    mValue.uPtr = other;
    mType = Nda::Reference;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::toBool(bool *ok) const
{
    if (ok) *ok = false;
    switch (mType) {
    case Nda::Undefined: return false; break;
    case Nda::Reference: return cInternalReference()->toBool(ok);
    case Nda::Any:       return false; break;
    case Nda::Number:    return false; break;
    case Nda::Natural:
        if (ok) *ok = true;
        return mValue.uInt64 != 0;
        break;
    case Nda::Supernatural:
        if (ok) *ok = true;
        return mValue.uInt64 != 0;
        break;
    case Nda::Boolean:
        if (ok) *ok = true;
        return mValue.uByte != 0;
        break;
    case Nda::Byte:
        if (ok) *ok = true;
        return (bool)mValue.uByte;
        break;
    case Nda::Character: return false; break;
    case Nda::String:    return false; break;
    case Nda::Struct:    return false; break;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
int64_t NdaVariant::toInt64(bool *ok) const
{
    if (ok) *ok = false;
    switch (mType) {
    case Nda::Undefined: return 0; break;
    case Nda::Reference: return cInternalReference()->toInt64(ok);
    case Nda::Any:       return 0; break;
    case Nda::Number: {
        int ret;
        bool isInt = exact32BitInt(ret);
        if (isInt) {
            if (ok) *ok = true;
            return ret;
        }
        return 0;
    }
    case Nda::Natural:
        if (ok) *ok = true;
        return mValue.uInt64;
        break;
    case Nda::Supernatural:
        if (mValue.uInt64 >= 0) {
            if (ok) *ok = true;
            return (int64_t)mValue.uUInt64;
        }
        break;
    case Nda::Boolean:
        if (ok) *ok = true;
        return mValue.uByte;
        break;
    case Nda::Byte:
        if (ok) *ok = true;
        return (bool)mValue.uByte;
        break;
    case Nda::Character: return false; break;
    case Nda::String:    return false; break;
    case Nda::Struct:    return false; break;
    }
    return false;

}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::isNan() const
{
    if (mType == Nda::Reference)
        return cInternalReference()->isNan();
    return (mType == Nda::Number) && std::isnan(mValue.uDouble);
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::assign(const NdaVariant &other)
{
    if (this == &other)
        return true;

    switch (mType) {
    case Nda::Undefined: return false; break;
    case Nda::Reference: return internalReference()->assign(other);
        break;
    case Nda::Any: {
        assignOther(other);
        return true;
    } break;
    case Nda::Number: {
        if (other.mType == mType) {
            mValue.uDouble = other.cuValue()->uDouble;
            return true;
        }
        if (other.type() == Nda::Natural) {
            mValue.uDouble = (double)other.cuValue()->uInt64;
            return true;
        }
    } break;
    case Nda::Natural: {
        if (other.mType == mType) {
            mValue.uInt64 = other.cuValue()->uInt64;
            return true;
        }
    } break;
    case Nda::Supernatural: {
        if (other.mType == mType) {
            mValue.uUInt64 = other.cuValue()->uUInt64;
            return true;
        }
    } break;
    case Nda::Boolean: {
        if (other.mType == mType) {
            mValue.uByte = other.cuValue()->uByte;
            return true;
        }
    } break;
    case Nda::Byte: {
        if (other.mType == mType) {
            mValue.uByte = other.cuValue()->uByte;
            return true;
        }
    } break;
    case Nda::String: {
        if (other.mType == mType) {
            if (other.cuValue()->uPtr == mValue.uPtr)
                return true;

            reset();
            assignOtherString(other);
            return true;
        }
    } break;
    case Nda::List: {
        if (other.type() == mType) {
            if (other.cInternalList() == cInternalList())
                return true;
            reset();
            assignOtherList(other);
            return true;
        }
    } break;

    case Nda::Character: return false; break;
    case Nda::Struct:    return false; break;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::equal(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->equal(other, ok);


    bool done;
    if (ok) *ok = false;

    auto res = spaceship(other, &done);

    if (!done)
        return false;

    if (res.isNan()) {
        if (ok) *ok = true;
        return false;
    }

    if (ok) *ok = true;
    return res.toInt64() == 0;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::logicalAnd(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->logicalAnd(other, ok);

    if (ok) *ok = false;

    bool done;
    bool left = toBool(&done);
    if (!done)
        return false;

    bool right = other.toBool(&done);
    if (!done)
        return false;

    if (ok) *ok = true;
    return left && right;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::logicalOr(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->logicalOr(other, ok);

    if (ok) *ok = false;

    bool done;
    bool left = toBool(&done);
    if (!done)
        return false;

    bool right = other.toBool(&done);
    if (!done)
        return false;

    if (ok) *ok = true;
    return left || right;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::logicalXor(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->logicalXor(other, ok);

    if (ok) *ok = false;

    bool done;
    bool left = toBool(&done);
    if (!done)
        return false;

    bool right = other.toBool(&done);
    if (!done)
        return false;

    if (ok) *ok = true;
    return left != right;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::greaterThen(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->greaterThen(other, ok);

    bool done;
    if (ok) *ok = false;

    auto res = spaceship(other, &done);

    if (!done)
        return false;

    if (res.isNan()) {
        if (ok) *ok = true;
        return false;
    }

    if (ok) *ok = true;
    return res.toInt64() > 0;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::lessThen(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->lessThen(other, ok);

    bool done;
    if (ok) *ok = false;

    auto res = spaceship(other, &done);

    if (!done)
        return false;

    if (res.isNan()) {
        if (ok) *ok = true;
        return false;
    }

    if (ok) *ok = true;
    return res.toInt64() < 0;
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::spaceship(const NdaVariant &other, bool *ok) const
{

    if (ok) *ok = false;

    NdaVariant ret;

    switch (mType) {
    case Nda::Undefined: return NdaVariant();
    case Nda::Reference: return cInternalReference()->spaceship(other, ok);
    case Nda::Any:       return NdaVariant();
    case Nda::Number:
        if (std::isnan(mValue.uDouble) || (other.type() == Nda::Number && std::isnan(other.cuValue()->uDouble))) {
            if (ok) *ok = true;
            ret.fromDoubleNan();
            return ret;
        }
        if (other.type() != mType) {
            int v1, v2;
            bool v1IsInt = exact32BitInt(v1);
            bool v2IsInt = other.exact32BitInt(v2);
            if (v1IsInt && v2IsInt) {
                if (ok) *ok = true;
                ret.fromNumber(OP_SPACESHIP(v1,v2));
                return ret;
            }
            if (v2IsInt) {
                if (ok) *ok = true;
                ret.fromNumber(OP_SPACESHIP(mValue.uDouble,(double)v2));
                return ret;
            }
            double otherVal;
            if (other.exact64BitDbl(otherVal)) {
                if (ok) *ok = true;
                ret.fromNumber(OP_SPACESHIP(mValue.uDouble,otherVal));
                return ret;
            }
            return ret; // giving up...
        }
        if (ok) *ok = true;
        ret.fromNumber(OP_SPACESHIP(mValue.uDouble,other.cuValue()->uDouble));
        break;
    case Nda::Natural:
        if (other.type() != mType)
            return ret;
        if (ok) *ok = true;
        ret.fromNumber(OP_SPACESHIP(mValue.uInt64,other.cuValue()->uInt64));
        break;
    case Nda::Supernatural:
        if (other.type() != mType)
            return ret;
        if (ok) *ok = true;
        ret.fromNumber(OP_SPACESHIP(mValue.uUInt64,other.cuValue()->uUInt64));
        break;
    case Nda::Boolean:
    case Nda::Byte:
    case Nda::Character:
        if (other.type() != mType)
            return ret;
        if (ok) *ok = true;
        ret.fromNumber(OP_SPACESHIP(mValue.uByte,other.cuValue()->uByte));
        break;
    case Nda::String:
        if (other.type() != mType)
            return ret;
        if (ok) *ok = true;
        if (!mValue.uPtr && !other.cuValue()->uPtr)   // both are empty
            ret.fromNumber((int64_t)  0);
        else if (!mValue.uPtr)                    // I'm empty the other is not -> not greater
            ret.fromNumber((int64_t) -1);
        if (!other.cuValue()->uPtr)                //  I'm not empty the other is empty -> greater
            ret.fromNumber((int64_t) +1);
        else
            ret.fromNumber(OP_SPACESHIP(cInternalString()->cValue(),other.cInternalString()->cValue()));
        break;
    case Nda::Struct:
        assert(0 && "not implemented");
        return ret;
    }
    return ret;
}


//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::concat(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->concat(other, ok);

    if (ok) *ok = false;

    switch (mType) {
    case Nda::String: {
        NdaVariant ret;
        if (ok) *ok = true;
        if (other.type() == Nda::String)
            ret.fromString(cInternalString()->cValue() + other.cInternalString()->cValue());
        else
            ret.fromString(cInternalString()->cValue() + other.toString());
        return ret;
    } break;
    case Nda::List: {
        if (other.type() == Nda::List) {
            NdaVariant ret;
            ret.initType(Nda::List);

            const auto &myArray    = cInternalList()->cArray();
            const auto &otherArray = other.cInternalList()->cArray();
            auto &newArray = ret.internalList()->array();
            newArray = myArray;
            newArray.insert(newArray.end(), otherArray.begin(), otherArray.end());

            if (ok) *ok = true;
            return ret;
        }
    } break;
    case Nda::Struct: {
        assert(0 && "not yet implemented");
        return NdaVariant();
    } break;
    default:
        break;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::subtract(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->subtract(other, ok);

    if (ok) *ok = false;
    if (mType != other.mType)
        return NdaVariant();

    switch (mType) {
    case Nda::Number: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uDouble - other.cuValue()->uDouble);
        return ret;
    } break;
    case Nda::Natural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uInt64 - other.cuValue()->uInt64);
        return ret;
    } break;
    case Nda::Supernatural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uUInt64 - other.cuValue()->uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::add(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->add(other, ok);

    if (ok) *ok = false;
    if (mType != other.type())
        return NdaVariant();

    switch (mType) {
    case Nda::Number: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uDouble + other.cuValue()->uDouble);
        return ret;
    } break;
    case Nda::Natural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uInt64 + other.cuValue()->uInt64);
        return ret;
    } break;
    case Nda::Supernatural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uUInt64 + other.cuValue()->uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();

}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::modulo(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->modulo(other, ok);


    if (ok) *ok = false;
    if (mType != other.type())
        return NdaVariant();

    switch (mType) {
    case Nda::Natural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uInt64 % other.cuValue()->uInt64);
        return ret;
    } break;
    case Nda::Supernatural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uUInt64 % other.cuValue()->uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::multiply(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->multiply(other, ok);

    if (ok) *ok = false;
    if (mType != other.type())
        return NdaVariant();

    switch (mType) {
    case Nda::Number: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uDouble * other.cuValue()->uDouble);
        return ret;
    } break;
    case Nda::Natural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uInt64 * other.cuValue()->uInt64);
        return ret;
    } break;
    case Nda::Supernatural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uUInt64 * other.cuValue()->uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::division(const NdaVariant &other, bool *ok) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->division(other, ok);


    if (ok) *ok = false;
    if (mType != other.type())
        return NdaVariant();

    switch (mType) {
    case Nda::Number: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uDouble / other.cuValue()->uDouble);
        return ret;
    } break;
    case Nda::Natural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uInt64 / other.cuValue()->uInt64);
        return ret;
    } break;
    case Nda::Supernatural: {
        NdaVariant ret;
        if (ok) *ok = true;
        ret.fromNumber(mValue.uUInt64 / other.cuValue()->uUInt64);
        return ret;
    } break;
    default:
        break;
    }

    return NdaVariant();

}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::unaryOperator(const std::string &op, bool *ok) const
{
    if (mType == Nda::Reference) {
        return cInternalReference()->unaryOperator(op, ok);
    }

    if (op == "#") {
        if (ok) *ok = true;
        return lengthOperator();
    }

    if (ok) *ok = false;

    NdaVariant ret;
    switch (mType) {
    case Nda::Undefined: break;
    case Nda::Any:       break;
    case Nda::Number: {
        if (op == "+") {
            if (ok) *ok = true;
            return *this;
        }
        if (op == "-") {
            // FIXME: nan? inf?
            if (ok) *ok = true;
            ret.fromNumber(-mValue.uDouble);
        }
    } break;
    case Nda::Natural:{
        if (op == "+") {
            if (ok) *ok = true;
            return *this;
        }
        if (op == "-") {
            if (ok) *ok = true;
            ret.fromNumber(-mValue.uInt64);
        }
    } break;
    case Nda::Supernatural:{
        if (op == "+") {
            if (ok) *ok = true;
            return *this;
        }
    } break;
    case Nda::Boolean:
        break;
    case Nda::Byte:
        break;
    case Nda::Character:
        break;
    case Nda::String:
        break;
    case Nda::Struct:
        break;
    }

    return ret;
}

//-------------------------------------------------------------------------------------------------
NdaVariant NdaVariant::lengthOperator() const
{
    if (mType == Nda::Reference) {
        return cInternalReference()->lengthOperator();
    }

    int length = 0;
    switch (mType) {
    case Nda::Reference: break;
    case Nda::Undefined: break;
    case Nda::Any:
    case Nda::Number:
    case Nda::Natural:
    case Nda::Supernatural:
    case Nda::Boolean:
    case Nda::Byte:
    case Nda::Character:
        length = 1;
        break;
    case Nda::String:
        length = cInternalString() ? cInternalString()->cValue().length() : 0;
        break;
    case Nda::List:
        length = listSize();
        break;
    case Nda::Dict:
        assert(0 && "not implemented");
        break;
    case Nda::Struct:
        assert(0 && "not implemented");
        break;
    }

    NdaVariant ret;
    ret.fromNumber((int64_t)length);
    return ret;
}

//-------------------------------------------------------------------------------------------------
int NdaVariant::listSize() const
{
    assert(type() == Nda::List);
    if (mType == Nda::Reference)
        return cInternalReference()->listSize();

    if (!mValue.uPtr)
        return 0;

    return cInternalList()->cArray().size();
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::appendToList(const NdaVariant &value)
{
    assert(type() == Nda::List);
    if (mType == Nda::Reference)
        return internalReference()->appendToList(value);

    if (!mValue.uPtr)
        mValue.uPtr = new Nda::SharedList();
    else if (internalList()->refCount() > 1)
        detachList();

    internalList()->array().push_back(value);
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::insertIntoList(int index, const NdaVariant &value)
{
    assert(type() == Nda::List);
    if (mType == Nda::Reference)
        return internalReference()->insertIntoList(index, value);

    if (!mValue.uPtr)
        mValue.uPtr = new Nda::SharedList();
    else if (internalList()->refCount() > 1)
        detachList();

    auto &array = internalList()->array();

    if (index >= (int)array.size())
        appendToList(value);

    if (index < 0)
        index = 0;

    array.insert(array.begin() + index,value);
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::takeFromList(int index)
{
    assert(type() == Nda::List);
    if (mType == Nda::Reference)
        return internalReference()->takeFromList(index);

    if (index < 0)
        return;

    if (index >= listSize())
        return;

    if (internalList()->refCount() > 1)
        detachList();

    auto &array = internalList()->array();
    array.erase(array.begin() + index);
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaVariant::writeAccess(int index)
{
    assert(type() == Nda::List);
    if (mType == Nda::Reference)
        return internalReference()->writeAccess(index);

    assert(index >= 0);
    assert(index < listSize());

    if (internalList()->refCount() > 1)
        detachList();

    auto &array = internalList()->array();
    return array.at(index);
}

//-------------------------------------------------------------------------------------------------
const NdaVariant &NdaVariant::readAccess(int index) const
{
    assert(type() == Nda::List);
    if (mType == Nda::Reference)
        return cInternalReference()->readAccess(index);

    assert(index >= 0);
    assert(index < listSize());

    const auto &array = cInternalList()->cArray();
    return array.at(index);
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::containsInList(const NdaVariant &value) const
{
    assert(type() == Nda::List);
    if (mType == Nda::Reference)
        return cInternalReference()->containsInList(value);

    if (listSize() <= 0)
        return false;

    const auto &array = cInternalList()->cArray();
    return std::find(array.begin(), array.end(), value) != array.end();
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::assignOther(const NdaVariant &other)
{
    reset();
    switch (other.mType) {
    case Nda::Undefined: return;
    case Nda::Reference: {
        mType       = other.mType;
        mValue.uPtr = other.mValue.uPtr;
        return;
    } break;
    case Nda::Any:       return;
    case Nda::Number:
    case Nda::Natural:
    case Nda::Supernatural:
    case Nda::Boolean:
    case Nda::Byte:
    case Nda::Character: {
        mType = other.mType;
        mValue.uInt64 = other.cuValue()->uInt64;
        return;
    } break;
    case Nda::String:
        assignOtherString(other);
        return;
    case Nda::List:
        assignOtherList(other);
        return;
    case Nda::Struct:
        assert(0 && "not implemented");
        return;
    }
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::assignOtherString(const NdaVariant &other)
{
    assert(mType != Nda::Reference);

    if (mType == Nda::Undefined || mType == Nda::Any)
        mType = Nda::String;

    if (mType != Nda::String)
        return;

    if (mValue.uPtr) {
        internalString()->refCount();
        mValue.uPtr = nullptr;
    }

    if (!other.cuValue()->uPtr)
        return;

    mValue.uPtr = other.cuValue()->uPtr;
    internalString()->addRef();
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::assignOtherList(const NdaVariant &other)
{
    assert(mType       == Nda::Undefined);
    assert(mValue.uPtr == nullptr);

    mType = Nda::List;
    if (!other.mValue.uPtr)
        return;
    mValue.uPtr = other.mValue.uPtr;
    internalList()->addRef();
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::setString(const std::string &newValue)
{
    if (mType == Nda::Reference)
        return internalReference()->setString(newValue);

    if (mType != Nda::String)
        return false;
    if (newValue.empty() && !mValue.uPtr)  // nothing to to
        return true;
    if (newValue.empty() &&  mValue.uPtr){ // almost nothing to to
        internalString()->releaseRef();
        mValue.uPtr = nullptr;
        return true;
    }

    // same value.. nothing to to
    if (mValue.uPtr && (newValue == internalString()->cValue()))
        return true;

    if (mValue.uPtr) {
        if (internalString()->refCount() > 1) { // detach?
            internalString()->releaseRef();
            mValue.uPtr = new Nda::SharedString(newValue);
            return true;
        }
        internalString()->value() = newValue;
    } else {
        mValue.uPtr = new Nda::SharedString(newValue);
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
std::string NdaVariant::toString() const
{
    if (mType == Nda::Reference)
        return cInternalReference()->toString();


    std::ostringstream oss;
    switch (mType) {
    case Nda::Undefined: return "";
    case Nda::Reference: return cInternalReference()->toString();
    case Nda::Any:       return "";
    case Nda::Number:
        oss << std::setprecision(15) << std::fixed << mValue.uDouble;
        break;
    case Nda::Natural:
        oss << mValue.uInt64;
        break;
    case Nda::Supernatural:
        oss << mValue.uUInt64;
        break;
    case Nda::Boolean:
    case Nda::Byte:
    case Nda::Character:
        return "NOT IMPLEMENTED";
    case Nda::String:
        if (mValue.uPtr)
            return cInternalString()->cValue();
        return "";
    case Nda::List:
        if (mValue.uPtr) {
            std::string ret;
            for (const auto &v : cInternalList()->cArray()) {
                if (!ret.empty())
                    ret = ret + ",";
                ret += v.toString();
            }
            ret = "[" + ret + "]";
            return ret;
        }

        return "[]";
    case Nda::Struct:
        return "NOT IMPLEMENTED";
    }

    return oss.str();
}

//-------------------------------------------------------------------------------------------------
Nda::Type NdaVariant::type() const
{
    if (mType == Nda::Reference)
        return cInternalReference()->type();

    return mType;
}

//-------------------------------------------------------------------------------------------------
Nda::Type NdaVariant::myType() const
{
    return mType;
}

//-------------------------------------------------------------------------------------------------
NdaVariant &NdaVariant::operator=(const NdaVariant &other)
{
    assignOther(other);
    return *this;
}

//-------------------------------------------------------------------------------------------------
int NdaVariant::refCount() const
{
    if (!mValue.uPtr)
        return 0;

    if (mType == Nda::String)
        return cInternalString()->refCount();

    assert(0 && "Not Implemented");
    return 0;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::fromNumber(const std::string &value, int64_t &ret)
{
    std::string cleanLiteral = NadaNumericParser::removeSeparators(value);

    ret = std::stoll(cleanLiteral, nullptr, 0);

    return true;
}


//-------------------------------------------------------------------------------------------------
void NdaVariant::reset()
{
    switch (mType) {
    case Nda::Undefined: return;
    case Nda::Any:
    case Nda::Number:
    case Nda::Natural:
    case Nda::Supernatural:
    case Nda::Boolean:
    case Nda::Byte:
    case Nda::Character:
        mType = Nda::Undefined;
        mValue.uInt64 = 0;
        return;
    case Nda::String:
        if (mValue.uPtr)
            ((Nda::SharedString*)mValue.uPtr)->releaseRef();
        mType = Nda::Undefined;
        mValue.uPtr = nullptr;
        return;
    case Nda::Reference:
        mType = Nda::Undefined;
        mValue.uPtr = nullptr;
        return;
    case Nda::List:
        if (mValue.uPtr)
            ((Nda::SharedList*)mValue.uPtr)->releaseRef();
        mType = Nda::Undefined;
        mValue.uPtr = nullptr;
        return;
    case Nda::Struct:
        assert(0 && "not implemented");
        return;
    }

    assert(0 && "Don't reach code here");
}

//-------------------------------------------------------------------------------------------------
Nda::SharedString *NdaVariant::internalString()
{
    assert(mType == Nda::String);
    assert(mValue.uPtr);
    return ((Nda::SharedString*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
const Nda::SharedString *NdaVariant::cInternalString() const
{
    assert(mType == Nda::String);
    return ((Nda::SharedString*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
NdaVariant *NdaVariant::internalReference()
{
    assert(mType == Nda::Reference);
    assert(mValue.uPtr);
    return ((NdaVariant*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
const NdaVariant *NdaVariant::cInternalReference() const
{
    assert(mType == Nda::Reference);
    assert(mValue.uPtr);
    return ((NdaVariant*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
Nda::SharedList *NdaVariant::internalList()
{
    if (mType == Nda::Reference)
        return internalReference()->internalList();

    assert(mType == Nda::List);
    if (!mValue.uPtr)
        mValue.uPtr = new Nda::SharedList();
    return ((Nda::SharedList*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
const Nda::SharedList   *NdaVariant::cInternalList() const
{
    if (mType == Nda::Reference)
        return cInternalReference()->cInternalList();

    assert(mType == Nda::List);
    return ((Nda::SharedList*)mValue.uPtr);
}

//-------------------------------------------------------------------------------------------------
void NdaVariant::detachList()
{
    assert(mType == Nda::List);
    assert(mValue.uPtr);
    assert(internalList()->refCount() >= 2);

    auto *newList = new Nda::SharedList();
    newList->array() = internalList()->array(); // deep copy
    internalList()->releaseRef();
    mValue.uPtr = newList;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::exact32BitInt(int &value) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->exact32BitInt(value);

    if ((mType == Nda::Natural) && (mValue.uInt64 <= INT32_MAX) && (mValue.uInt64 >= INT32_MIN))
    {
        value = (int)mValue.uInt64;
        return true;
    }

    if ((mType == Nda::Supernatural) && (mValue.uUInt64 <= INT32_MAX))
    {
        value = (int)mValue.uUInt64;
        return true;
    }

    if ((mType == Nda::Number) && (std::floor(mValue.uDouble) == mValue.uDouble)
        && (mValue.uDouble >= INT32_MIN && mValue.uDouble <= INT32_MAX))
    {
        value = static_cast<int>(mValue.uDouble);
        return true;
    }

    if (mType == Nda::Byte) {
        value = mValue.uByte;
        return true;
    }

    if (mType == Nda::Character) {
        value = mValue.uChar;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
bool NdaVariant::exact64BitDbl(double &value) const
{
    if (mType == Nda::Reference)
        return cInternalReference()->exact64BitDbl(value);

    switch (mType) {
    case Nda::Undefined:
    case Nda::Reference:
    case Nda::Any:
    case Nda::String:
    case Nda::Struct:
        return false;

    case Nda::Number: {
        value = mValue.uDouble;
        return true;
    } break;
    case Nda::Natural: {
        if (mValue.uInt64 >= -((int64_t(1) << 53) - 1) && mValue.uInt64 <= ((int64_t(1) << 53) - 1))
            value = static_cast<double>(mValue.uInt64);
        return true;
    } break;

    case Nda::Supernatural: {
        if (mValue.uInt64 >= 0 && mValue.uInt64 <= ((int64_t(1) << 53) - 1))
            value = static_cast<double>(mValue.uUInt64);
        return true;
    } break;
    case Nda::Boolean:
    case Nda::Byte:
        value = static_cast<double>(mValue.uByte);
        return true;
    case Nda::Character:
        value = static_cast<double>(mValue.uChar);
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//                                   local functions
//-------------------------------------------------------------------------------------------------

bool       operator==(const NdaVariant &v1, const NdaVariant &v2)
{
    if (v1.type() != v2.type())
        return false;

    return v1.equal(v2);
}

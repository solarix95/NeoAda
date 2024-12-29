#include "type.h"
#include "private/utils.h"

//-------------------------------------------------------------------------------------------------
Nda::Type Nda::typeByString(const std::string &name)
{
    std::string lowerName = Nda::toLower(name);

    if (lowerName == "any")
        return Nda::Any;

    if (lowerName == "number")
        return Nda::Number;

    if (lowerName == "natural")
        return Nda::Natural;

    if (lowerName == "supernatural")
        return Nda::Supernatural;

    if (lowerName == "boolean")
        return Nda::Boolean;

    if (lowerName == "byte")
        return Nda::Byte;

    if (lowerName == "character")
        return Nda::Character;

    if (lowerName == "string")
        return Nda::String;

    if (lowerName == "struct")
        return Nda::Struct;

    return Nda::Undefined;
}

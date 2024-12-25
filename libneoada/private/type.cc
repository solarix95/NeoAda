#include "type.h"
#include "private/utils.h"

//-------------------------------------------------------------------------------------------------
Nada::Type Nada::typeByString(const std::string &name)
{
    std::string lowerName = Nada::toLower(name);

    if (lowerName == "any")
        return Nada::Any;

    if (lowerName == "number")
        return Nada::Number;

    if (lowerName == "natural")
        return Nada::Natural;

    if (lowerName == "supernatural")
        return Nada::Supernatural;

    if (lowerName == "boolean")
        return Nada::Boolean;

    if (lowerName == "byte")
        return Nada::Byte;

    if (lowerName == "character")
        return Nada::Character;

    if (lowerName == "string")
        return Nada::String;

    if (lowerName == "struct")
        return Nada::Struct;

    return Nada::Undefined;
}

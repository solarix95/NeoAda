#ifndef TYPE_H
#define TYPE_H

#include <string>

namespace Nada {
    enum Type { Undefined, Reference, Any, Number, Natural, Supernatural, Boolean, Byte, Character, String, Struct };

    Type typeByString(const std::string &name);
}






#endif // TYPE_H

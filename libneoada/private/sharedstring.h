#ifndef SHAREDSTRING_H
#define SHAREDSTRING_H

#include <string>
#include "shareddata.h"

namespace Nda {


class SharedString : public Nda::SharedData
{
public:
    SharedString(const std::string &value);

    inline std::string        &value()        { return mValue; }
    inline const std::string  &cValue() const { return mValue; }

private:
    std::string  mValue;

};

}

#endif // SHAREDSTRING_H

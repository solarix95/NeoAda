#include <algorithm>
#include "utils.h"

//-------------------------------------------------------------------------------------------------
std::string Nada::toLower(const std::string &name)
{
    std::string ret = name;

    std::transform(ret.begin(), ret.end(), ret.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    return ret;
}

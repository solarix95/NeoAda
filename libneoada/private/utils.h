#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace Nda {
    std::string toLower(const std::string &name);

struct LowerString {
    std::string displayValue;
    std::string lowerValue;

    LowerString(const std::string &n = "") : displayValue(n), lowerValue(toLower(n)) {}

    LowerString& operator=(const std::string& n) {
        displayValue = n;
        lowerValue   = toLower(n);
        return *this; // Ermöglicht a = b = c
    }
};
}

#endif // UTILS_H

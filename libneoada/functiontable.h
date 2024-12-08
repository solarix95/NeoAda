#ifndef FUNCTIONTABLE_H
#define FUNCTIONTABLE_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "value.h"

using NadaFncParameters = std::vector<std::pair<std::string, std::string>>;
using NadaFncValues     = std::unordered_map<std::string, NadaValue>;
using NadaFncCallback   = std::function<NadaValue(const NadaFncValues&)>;

struct NadaFunctionEntry {
    std::string       returnType;
    NadaFncParameters parameters;

    NadaFncCallback   nativeCallback; // Built-in

    NadaFncValues     fncValues(const NadaValues &values) const;
};

struct NadaOverloadedFunction {
    std::string functionName;
    std::vector<NadaFunctionEntry> overloads;
};

class NadaFunctionTable
{
public:
    NadaFunctionTable();

    // Init/Setup
    bool              bind(const std::string &name, const NadaFncParameters &parameters, NadaFncCallback cb); // c++ callback :)

    // Runtime
    bool              contains(const std::string &name, const NadaValues &parameters);
    NadaFunctionEntry &symbol(const std::string &name, const NadaValues &parameters);

private:
    std::unordered_map<std::string, NadaOverloadedFunction> mFunctions;
};

#endif // FUNCTIONTABLE_H

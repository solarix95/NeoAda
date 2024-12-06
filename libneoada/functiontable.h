#ifndef FUNCTIONTABLE_H
#define FUNCTIONTABLE_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "value.h"

using NadaFncParameters = std::vector<std::pair<std::string, std::string>>;
using NadaFncCallback   = std::function<NadaValue(const NadaFncParameters&)>;

struct NadaFunctionEntry {
    std::string       returnType;
    NadaFncParameters parameters;

    NadaFncCallback   nativeCallback; // Built-in
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

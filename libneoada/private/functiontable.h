#ifndef FUNCTIONTABLE_H
#define FUNCTIONTABLE_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "variant.h"
#include "parser.h"

namespace Nda {



enum ParameterMode {
    InMode,
    OutMode
};


struct FormalParameter {
    std::string   name;
    std::string   type;
    ParameterMode mode;
};


using FncParameters = std::vector<FormalParameter>;
using FncValues     = std::unordered_map<std::string, NdaVariant>;
using FncCallback   = std::function<bool (const FncValues&, NdaVariant &ret)>;
using PrcCallback   = std::function<bool (const FncValues&)>;

struct FunctionEntry {
    std::string        returnType;
    FncParameters parameters;

    std::shared_ptr<NadaParser::ASTNode> block;          // NeoAda-Code
    FncCallback                     nativeFncCallback;   // c++ Built-in
    PrcCallback                     nativePrcCallback;   // c++ Built-in

    FncValues     fncValues(const NadaValues &values) const;
};

struct OverloadedFunction {
    std::string functionName;
    std::vector<FunctionEntry> overloads;
};


}





class NadaFunctionTable
{
public:
    NadaFunctionTable();

    // Init/Setup
    bool              bindFnc(const std::string &name, const Nda::FncParameters &parameters, Nda::FncCallback cb); // c++ function  callback
    bool              bindPrc(const std::string &name, const Nda::FncParameters &parameters, Nda::PrcCallback cb); // c++ procedure callback

    bool              bind(const std::string &name, const Nda::FncParameters &parameters, const NadaParser::ASTNodePtr &block);

    // Runtime
    bool               contains(const std::string &name, const NadaValues &parameters);
    Nda::FunctionEntry &symbol(const std::string &name, const NadaValues &parameters);

private:
    std::unordered_map<std::string, Nda::OverloadedFunction> mFunctions;
};

#endif // FUNCTIONTABLE_H

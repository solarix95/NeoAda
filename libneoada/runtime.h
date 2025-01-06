#ifndef LIB_NEOADA_RUNTIME_H
#define LIB_NEOADA_RUNTIME_H

#include <string>
#include "variant.h"

class NdaException;
class NdaState;

class NdaRuntime
{
public:
    NdaRuntime();
    virtual ~NdaRuntime();

    void       reset();
    NdaVariant runScript(const std::string &script, NdaException *e = nullptr);
    NdaVariant runFile(const std::string &fileName, NdaException *e = nullptr);
    NdaState  *state();

    void loadAddonAdaString();
    void loadAddonAdaList();



private:
    void destroy();

    NdaState *mState;
};

#endif // NDARUNTIME_H

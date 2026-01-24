#ifndef LIB_NEOADA_RUNTIME_H
#define LIB_NEOADA_RUNTIME_H

#include <string>
#include "variant.h"
#include "value.h"

class NdaException;
class NdaState;
class NdaInterpreter;

class NdaRuntime
{
public:
    NdaRuntime();
    virtual ~NdaRuntime();

    void        reset();
    bool        hasError() const;
    std::string lastError() const;

    NdaVariant runScript(const std::string &script, NdaException *e = nullptr);
    NdaVariant runFile(const std::string &fileName, NdaException *e = nullptr);
    NdaState  *state();

    NdaValue   invokeFnc(const std::string &fncName);    // function fncName(arg1: any) return any;
    NdaValue   invokeFnc(const std::string &fncName, const NdaValue &arg1);    // function fncName(arg1: any) return any;
    NdaValue   invokeFnc(const std::string &fncName, const NdaValue &arg1, const NdaValue &arg2);

    void       invokePrc(const std::string &prcName, const NdaValue &arg1);    // function fncName(arg1: any) return any;

    virtual void loadAddonAdaString();
    virtual void loadAddonAdaList();
    virtual void loadAddonAdaIoFile();

private:
    void destroy();

    NdaState        *mState;
    NdaInterpreter  *mInterpreter;

    std::string      mLastError;
};

#endif // NDARUNTIME_H

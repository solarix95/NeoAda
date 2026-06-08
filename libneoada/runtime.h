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
    std::vector<std::string> globalFunctions() const;

    NdaValue   invokeFnc(const std::string &fncName);    // function fncName(arg1: any) return any;
    NdaValue   invokeFnc(const std::string &fncName, const NdaValue &arg1);    // function fncName(arg1: any) return any;
    NdaValue   invokeFnc(const std::string &fncName, const NdaValue &arg1, const NdaValue &arg2);

    void       invokePrc(const std::string &prcName);
    void       invokePrc(const std::string &prcName, const NdaValue &arg1);
    void       invokePrc(const std::string &prcName, const NdaValue &arg1, const NdaValue &arg2,
                         const NdaValue &arg3, const NdaValue &arg4);
    void       invokePrc(const std::string &prcName, const NdaValue &arg1, const NdaValue &arg2,
                         const NdaValue &arg3, const NdaValue &arg4, const NdaValue &arg5);

    virtual void loadAddonAdaString();
    virtual void loadAddonAdaList();
    virtual void loadAddonAdaDict();
    virtual void loadAddonAdaBytes();
    virtual void loadAddonAdaMath();
    virtual void loadAddonAdaIoFile();
    virtual void loadAddonAdaTextEncoding();
    virtual void loadAddonAdaDateTime();
    virtual void loadAddonAdaRegexp();
    virtual void loadAddonAdaJson();

private:
    void destroy();

    NdaState        *mState;
    NdaInterpreter  *mInterpreter;

    std::string      mLastError;
};

#endif // NDARUNTIME_H

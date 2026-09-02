#ifndef LIB_NEOADA_FORMULA_H
#define LIB_NEOADA_FORMULA_H

namespace Nda {
struct Runnable;
}

class NdaFormula
{
public:
    NdaFormula();
    ~NdaFormula();

    NdaFormula(NdaFormula &&other);
    NdaFormula &operator=(NdaFormula &&other);

    NdaFormula(const NdaFormula &) = delete;
    NdaFormula &operator=(const NdaFormula &) = delete;

    bool isValid() const;
    Nda::Runnable *runnable() const;
    explicit NdaFormula(Nda::Runnable *runnable);

private:
    friend class NdaRuntime;
    friend class NdaInterpreter;

    Nda::Runnable *mRunnable;
};

#endif

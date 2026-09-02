#include "formula.h"
#include "private/runnable.h"

//-------------------------------------------------------------------------------------------------
NdaFormula::NdaFormula()
    : mRunnable(nullptr)
{
}

//-------------------------------------------------------------------------------------------------
NdaFormula::NdaFormula(Nda::Runnable *runnable)
    : mRunnable(runnable)
{
}

//-------------------------------------------------------------------------------------------------
NdaFormula::~NdaFormula()
{
    delete mRunnable;
}

//-------------------------------------------------------------------------------------------------
NdaFormula::NdaFormula(NdaFormula &&other)
    : mRunnable(other.mRunnable)
{
    other.mRunnable = nullptr;
}

//-------------------------------------------------------------------------------------------------
NdaFormula &NdaFormula::operator=(NdaFormula &&other)
{
    if (this != &other) {
        delete mRunnable;
        mRunnable = other.mRunnable;
        other.mRunnable = nullptr;
    }
    return *this;
}

//-------------------------------------------------------------------------------------------------
bool NdaFormula::isValid() const
{
    return mRunnable != nullptr;
}

//-------------------------------------------------------------------------------------------------
Nda::Runnable *NdaFormula::runnable() const
{
    return mRunnable;
}

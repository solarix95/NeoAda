#ifndef ABSTRACTSCENARIO_H
#define ABSTRACTSCENARIO_H

#include <QString>

class NdaRuntime;
class QWidget;

class AbstractScenario
{
public:
    virtual ~AbstractScenario() = default;

    virtual QString packageName() const = 0;
    virtual QString title() const = 0;
    virtual QString description() const = 0;
    virtual QString initialSource() const = 0;

    virtual QWidget *widget() = 0;
    virtual void bind(NdaRuntime &runtime) = 0;
    virtual void reset() = 0;
    virtual void afterRun(NdaRuntime &runtime) { (void)runtime; }
    virtual void stop() {}
};

#endif // ABSTRACTSCENARIO_H

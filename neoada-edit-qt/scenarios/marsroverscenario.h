#ifndef MARSROVERSCENARIO_H
#define MARSROVERSCENARIO_H

#include "abstractscenario.h"

#include <QString>
#include <QVector>
#include <QWidget>

class QPushButton;
class QTimer;

class MarsRoverWidget : public QWidget
{
public:
    explicit MarsRoverWidget(QWidget *parent = nullptr);

    void reset();
    void start();
    void stop();
    void forward();
    void backward();
    void steerLeft();
    void steerRight();
    void animateRadar();
    bool isRunning() const;
    QString sensorValue() const;
    double x() const;
    double y() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    enum Direction { North, East, South, West };
    struct Rock { double x; double y; double rx; double ry; };

    QRect playFieldRect() const;
    void loadDefaultMap();
    void randomizeMap();

    double mX;
    double mY;
    Direction mDirection;
    bool mRunning;
    double mWheelPhase;
    double mRadarPhase;
    QVector<Rock> mRocks;
    double mTargetX;
    double mTargetY;
    QPushButton *mRandomButton;
};

class MarsRoverScenario : public AbstractScenario
{
public:
    MarsRoverScenario();
    ~MarsRoverScenario() override;

    QString packageName() const override;
    QString title() const override;
    QString description() const override;
    QString initialSource() const override;

    QWidget *widget() override;
    void bind(NdaRuntime &runtime) override;
    void reset() override;
    void afterRun(NdaRuntime &runtime) override;
    void stop() override;

private:
    void tick();

    MarsRoverWidget *mWidget;
    QTimer *mTimer;
    NdaRuntime *mRuntime;
};

#endif // MARSROVERSCENARIO_H

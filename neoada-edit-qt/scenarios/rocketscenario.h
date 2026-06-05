#ifndef ROCKETSCENARIO_H
#define ROCKETSCENARIO_H

#include "abstractscenario.h"

#include <QPointF>
#include <QString>
#include <QVector>
#include <QWidget>

class QTimer;

class RocketWidget : public QWidget
{
public:
    explicit RocketWidget(QWidget *parent = nullptr);

    void reset();
    void start();
    void stop();
    void steerLeft();
    void steerRight();
    void stage();
    void simulationStep();

    bool isRunning() const;
    double x() const;
    double y() const;
    double height() const;
    double fuel() const;
    double speed() const;
    double missionTime() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QPointF worldToScreen(double x, double y) const;
    void drawPlanet(QPainter &p) const;
    void drawRocket(QPainter &p) const;

    double mX;
    double mY;
    double mVx;
    double mVy;
    double mAngle;
    int mStage;
    bool mRunning;
    double mFuel;
    double mFlamePhase;
    double mMissionTime;
    QVector<QPointF> mTrajectory;
    double mLaunchZoom;
    double mOrbitZoom;
};

class RocketScenario : public AbstractScenario
{
public:
    RocketScenario();
    ~RocketScenario() override;

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
    void showRuntimeError();

    RocketWidget *mWidget;
    QTimer *mTimer;
    NdaRuntime *mRuntime;
};

#endif // ROCKETSCENARIO_H

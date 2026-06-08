#ifndef ASTEROIDDEFENSESCENARIO_H
#define ASTEROIDDEFENSESCENARIO_H

#include "abstractscenario.h"

#include <QPointF>
#include <QVector>
#include <QWidget>

class NdaRuntime;
class QTimer;

class AsteroidDefenseWidget : public QWidget
{
public:
    struct Detection {
        qint64 id;
        double x;
        double y;
        double dx;
        double dy;
    };

    explicit AsteroidDefenseWidget(QWidget *parent = nullptr);

    void reset();
    void start();
    void stop();
    void fireStart();
    void fireStop();
    void setTargetAngle(double angle);
    void simulationStep();

    bool isRunning() const;
    bool isFinished() const;
    double currentAngle() const;
    double heatLevel() const;
    QVector<Detection> takeSpawnedAsteroids();
    QVector<qint64> takeDestroyedAsteroids();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Asteroid {
        qint64 id;
        QPointF position;
        QPointF velocity;
        double radius;
        double rotation;
    };
    struct HitParticle {
        QPointF position;
        QPointF velocity;
        double life;
        double maxLife;
        double size;
        int colorIndex;
    };
    struct Projectile {
        QPointF position;
        QPointF velocity;
        double life;
    };

    QRectF playField() const;
    QPointF cannonPosition() const;
    void spawnAsteroid();
    void fireProjectile();
    void spawnHitEffect(const QPointF &position, double asteroidRadius);
    void resolveCollisions();

    QVector<Asteroid> mAsteroids;
    QVector<Projectile> mProjectiles;
    QVector<HitParticle> mHitParticles;
    QVector<Detection> mSpawnedAsteroids;
    QVector<qint64> mDestroyedAsteroids;
    qint64 mNextAsteroidId;
    double mTargetAngle;
    double mCurrentAngle;
    double mSpawnAccumulator;
    double mFireAccumulator;
    double mMissionTime;
    double mHeatLevel;
    bool mRunning;
    bool mFiring;
    bool mLeftHouseAlive;
    bool mRightHouseAlive;
    bool mCannonAlive;
};

class AsteroidDefenseScenario : public AbstractScenario
{
public:
    AsteroidDefenseScenario();
    ~AsteroidDefenseScenario() override;

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

    AsteroidDefenseWidget *mWidget;
    QTimer *mTimer;
    NdaRuntime *mRuntime;
    double mRunAccumulator;
};

#endif // ASTEROIDDEFENSESCENARIO_H

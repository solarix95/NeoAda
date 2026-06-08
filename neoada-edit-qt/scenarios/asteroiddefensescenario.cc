#include "asteroiddefensescenario.h"

#include <QtMath>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QTimer>

#include <runtime.h>
#include <state.h>

namespace {
constexpr double kDt = 0.04;
constexpr double kWorldHalfWidth = 100.0;
constexpr double kWorldHeight = 110.0;
constexpr double kHouseHalfWidth = 6.0;
constexpr double kHouseHeight = 12.0;
constexpr double kCannonBaseRadius = 5.0;
constexpr double kCannonBarrelLength = 10.0;
constexpr double kCannonBarrelRadius = 1.5;

static double distanceSquared(const QPointF &a, const QPointF &b)
{
    const double dx = a.x() - b.x();
    const double dy = a.y() - b.y();
    return dx * dx + dy * dy;
}

static bool circleIntersectsRect(const QPointF &center, double radius, const QRectF &rect)
{
    const double closestX = qBound(rect.left(), center.x(), rect.right());
    const double closestY = qBound(rect.top(), center.y(), rect.bottom());
    return distanceSquared(center, QPointF(closestX, closestY)) <= radius * radius;
}

static double distanceToSegmentSquared(const QPointF &point, const QPointF &start, const QPointF &end)
{
    const QPointF segment = end - start;
    const double lengthSquared = QPointF::dotProduct(segment, segment);
    if (lengthSquared <= 0.0)
        return distanceSquared(point, start);

    const double t = qBound(0.0, QPointF::dotProduct(point - start, segment) / lengthSquared, 1.0);
    return distanceSquared(point, start + segment * t);
}

static QColor blendedColor(const QColor &cold, const QColor &hot, double amount)
{
    const double t = qBound(0.0, amount, 1.0);
    return QColor(int(cold.red() + (hot.red() - cold.red()) * t),
                  int(cold.green() + (hot.green() - cold.green()) * t),
                  int(cold.blue() + (hot.blue() - cold.blue()) * t));
}
}

AsteroidDefenseWidget::AsteroidDefenseWidget(QWidget *parent)
    : QWidget(parent)
    , mNextAsteroidId(1)
    , mTargetAngle(0.0)
    , mCurrentAngle(0.0)
    , mSpawnAccumulator(0.0)
    , mFireAccumulator(0.0)
    , mMissionTime(0.0)
    , mHeatLevel(0.0)
    , mRunning(false)
    , mFiring(false)
    , mLeftHouseAlive(true)
    , mRightHouseAlive(true)
    , mCannonAlive(true)
{
    setMinimumSize(680, 430);
    setAutoFillBackground(true);
}

void AsteroidDefenseWidget::reset()
{
    mAsteroids.clear();
    mProjectiles.clear();
    mHitParticles.clear();
    mSpawnedAsteroids.clear();
    mDestroyedAsteroids.clear();
    mNextAsteroidId = 1;
    mTargetAngle = 0.0;
    mCurrentAngle = 0.0;
    mSpawnAccumulator = 0.0;
    mFireAccumulator = 0.0;
    mMissionTime = 0.0;
    mHeatLevel = 0.0;
    mRunning = false;
    mFiring = false;
    mLeftHouseAlive = true;
    mRightHouseAlive = true;
    mCannonAlive = true;
    update();
}

void AsteroidDefenseWidget::start()
{
    if (!isFinished())
        mRunning = true;
    update();
}

void AsteroidDefenseWidget::stop()
{
    mRunning = false;
    mFiring = false;
    update();
}

void AsteroidDefenseWidget::fireStart()
{
    if (mCannonAlive && mHeatLevel < 100.0)
        mFiring = true;
}

void AsteroidDefenseWidget::fireStop()
{
    mFiring = false;
}

void AsteroidDefenseWidget::setTargetAngle(double angle)
{
    mTargetAngle = qBound(-90.0, angle, 90.0);
}

bool AsteroidDefenseWidget::isRunning() const
{
    return mRunning;
}

bool AsteroidDefenseWidget::isFinished() const
{
    return !mCannonAlive || (!mLeftHouseAlive && !mRightHouseAlive);
}

double AsteroidDefenseWidget::currentAngle() const
{
    return mCurrentAngle;
}

double AsteroidDefenseWidget::heatLevel() const
{
    return mHeatLevel;
}

QVector<AsteroidDefenseWidget::Detection> AsteroidDefenseWidget::takeSpawnedAsteroids()
{
    QVector<Detection> events;
    events.swap(mSpawnedAsteroids);
    return events;
}

QVector<qint64> AsteroidDefenseWidget::takeDestroyedAsteroids()
{
    QVector<qint64> events;
    events.swap(mDestroyedAsteroids);
    return events;
}

QRectF AsteroidDefenseWidget::playField() const
{
    return rect().adjusted(12, 12, -12, -12);
}

QPointF AsteroidDefenseWidget::cannonPosition() const
{
    return QPointF(0.0, 3.4);
}

void AsteroidDefenseWidget::spawnAsteroid()
{
    auto *rng = QRandomGenerator::global();
    Asteroid asteroid;
    asteroid.id = mNextAsteroidId++;
    asteroid.position = QPointF(rng->bounded(-92, 93), kWorldHeight + rng->bounded(5, 28));
    asteroid.velocity = QPointF(rng->bounded(-50, 51) / 10.0, -8.0 - rng->bounded(75) / 10.0);
    asteroid.radius = 3.2 + rng->bounded(35) / 10.0;
    asteroid.rotation = rng->bounded(3600) / 10.0;
    mAsteroids.push_back(asteroid);
    mSpawnedAsteroids.push_back({asteroid.id, asteroid.position.x(), asteroid.position.y(),
                                 asteroid.velocity.x(), asteroid.velocity.y()});
}

void AsteroidDefenseWidget::fireProjectile()
{
    if (!mCannonAlive)
        return;

    const double angle = qDegreesToRadians(mCurrentAngle);
    const QPointF direction(qSin(angle), qCos(angle));
    Projectile projectile;
    projectile.position = cannonPosition() + direction * 9.0;
    projectile.velocity = direction * 88.0;
    projectile.life = 2.2;
    mProjectiles.push_back(projectile);
}

void AsteroidDefenseWidget::spawnHitEffect(const QPointF &position, double asteroidRadius)
{
    auto *rng = QRandomGenerator::global();
    const int particleCount = 14 + rng->bounded(7);
    for (int i = 0; i < particleCount; ++i) {
        const double angle = qDegreesToRadians(rng->bounded(3600) / 10.0);
        const double speed = 13.0 + rng->bounded(220) / 10.0 + asteroidRadius;
        HitParticle particle;
        particle.position = position;
        particle.velocity = QPointF(qCos(angle) * speed, qSin(angle) * speed);
        particle.life = 0.45 + rng->bounded(35) / 100.0;
        particle.maxLife = particle.life;
        particle.size = 1.5 + rng->bounded(25) / 10.0;
        particle.colorIndex = rng->bounded(4);
        mHitParticles.push_back(particle);
    }
}

void AsteroidDefenseWidget::simulationStep()
{
    if (!mRunning || isFinished())
        return;

    mMissionTime += kDt;
    mSpawnAccumulator += kDt;
    mFireAccumulator += kDt;

    if (mFiring)
        mHeatLevel = qMin(100.0, mHeatLevel + 18.0 * kDt);
    else
        mHeatLevel = qMax(0.0, mHeatLevel - 11.0 * kDt);
    if (mHeatLevel >= 100.0)
        mFiring = false;

    const double spawnInterval = qMax(0.42, 1.45 - mMissionTime * 0.006);
    if (mSpawnAccumulator >= spawnInterval) {
        mSpawnAccumulator = 0.0;
        spawnAsteroid();
    }

    const double turnStep = 150.0 * kDt;
    if (mCurrentAngle < mTargetAngle)
        mCurrentAngle = qMin(mTargetAngle, mCurrentAngle + turnStep);
    else if (mCurrentAngle > mTargetAngle)
        mCurrentAngle = qMax(mTargetAngle, mCurrentAngle - turnStep);

    if (mFiring && mFireAccumulator >= 0.16) {
        mFireAccumulator = 0.0;
        fireProjectile();
    }

    for (auto &asteroid : mAsteroids) {
        asteroid.position += asteroid.velocity * kDt;
        asteroid.rotation += 50.0 * kDt;
    }
    for (auto &projectile : mProjectiles) {
        projectile.position += projectile.velocity * kDt;
        projectile.life -= kDt;
    }
    for (auto &particle : mHitParticles) {
        particle.position += particle.velocity * kDt;
        particle.velocity += QPointF(0.0, -28.0) * kDt;
        particle.life -= kDt;
    }

    resolveCollisions();

    for (int i = mProjectiles.size() - 1; i >= 0; --i) {
        if (mProjectiles[i].life <= 0.0 || mProjectiles[i].position.y() > kWorldHeight + 20.0
                || qAbs(mProjectiles[i].position.x()) > kWorldHalfWidth + 20.0)
            mProjectiles.remove(i);
    }
    for (int i = mHitParticles.size() - 1; i >= 0; --i) {
        if (mHitParticles[i].life <= 0.0)
            mHitParticles.remove(i);
    }
    for (int i = mAsteroids.size() - 1; i >= 0; --i) {
        if (mAsteroids[i].position.y() < -15.0) {
            mDestroyedAsteroids.push_back(mAsteroids[i].id);
            mAsteroids.remove(i);
        }
    }

    if (isFinished()) {
        mRunning = false;
        mFiring = false;
    }
    update();
}

void AsteroidDefenseWidget::resolveCollisions()
{
    for (int ai = mAsteroids.size() - 1; ai >= 0; --ai) {
        bool destroyed = false;
        for (int pi = mProjectiles.size() - 1; pi >= 0; --pi) {
            const double radius = mAsteroids[ai].radius + 1.4;
            if (distanceSquared(mAsteroids[ai].position, mProjectiles[pi].position) <= radius * radius) {
                spawnHitEffect(mAsteroids[ai].position, mAsteroids[ai].radius);
                mProjectiles.remove(pi);
                mDestroyedAsteroids.push_back(mAsteroids[ai].id);
                mAsteroids.remove(ai);
                destroyed = true;
                break;
            }
        }
        if (destroyed)
            continue;

        const Asteroid &asteroid = mAsteroids[ai];
        const QRectF leftHouse(-54.0 - kHouseHalfWidth, 0.0,
                               kHouseHalfWidth * 2.0, kHouseHeight);
        const QRectF rightHouse(54.0 - kHouseHalfWidth, 0.0,
                                kHouseHalfWidth * 2.0, kHouseHeight);

        const QPointF cannon = cannonPosition();
        const double angle = qDegreesToRadians(mCurrentAngle);
        const QPointF barrelDirection(qSin(angle), qCos(angle));
        const QPointF barrelEnd = cannon + barrelDirection * kCannonBarrelLength;
        const bool hitsCannonBase = distanceSquared(asteroid.position, cannon)
                <= qPow(asteroid.radius + kCannonBaseRadius, 2.0);
        const bool hitsCannonBarrel = distanceToSegmentSquared(asteroid.position, cannon, barrelEnd)
                <= qPow(asteroid.radius + kCannonBarrelRadius, 2.0);

        if (mCannonAlive && (hitsCannonBase || hitsCannonBarrel)) {
            mCannonAlive = false;
            mDestroyedAsteroids.push_back(asteroid.id);
            mAsteroids.remove(ai);
        } else if (mLeftHouseAlive
                   && circleIntersectsRect(asteroid.position, asteroid.radius, leftHouse)) {
            mLeftHouseAlive = false;
            mDestroyedAsteroids.push_back(asteroid.id);
            mAsteroids.remove(ai);
        } else if (mRightHouseAlive
                   && circleIntersectsRect(asteroid.position, asteroid.radius, rightHouse)) {
            mRightHouseAlive = false;
            mDestroyedAsteroids.push_back(asteroid.id);
            mAsteroids.remove(ai);
        }
    }
}

void AsteroidDefenseWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QRectF field = playField();
    p.fillRect(rect(), QColor(9, 13, 25));

    p.save();
    p.setClipRect(field);
    for (int i = 0; i < 75; ++i) {
        const int x = int(field.left()) + (i * 83 + 17) % qMax(1, int(field.width()));
        const int y = int(field.top()) + (i * 47 + 29) % qMax(1, int(field.height() * 0.78));
        p.setPen(QColor(220, 232, 255, 95 + (i * 31) % 150));
        p.drawPoint(x, y);
    }

    auto screen = [&](const QPointF &world) {
        const double sx = field.center().x() + world.x() / kWorldHalfWidth * field.width() * 0.48;
        const double sy = field.bottom() - 32.0 - world.y() / kWorldHeight * (field.height() - 65.0);
        return QPointF(sx, sy);
    };
    const double scale = field.width() / (kWorldHalfWidth * 2.0) * 0.96;
    const double groundY = screen(QPointF(0.0, 0.0)).y();

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(33, 100, 181));
    p.drawEllipse(QPointF(field.center().x(), groundY + 155.0), field.width() * 0.72, 185.0);
    p.setBrush(QColor(60, 145, 82));
    p.drawRect(QRectF(field.left(), groundY, field.width(), field.bottom() - groundY));

    auto drawHouse = [&](double x, bool alive) {
        const QPointF c = screen(QPointF(x, 3.0));
        if (!alive) {
            p.setPen(QPen(QColor(95, 78, 70), 3));
            p.drawLine(c + QPointF(-13, 8), c + QPointF(13, -8));
            p.drawLine(c + QPointF(-12, -4), c + QPointF(14, 9));
            return;
        }
        p.setPen(QPen(QColor(235, 221, 190), 2));
        p.setBrush(QColor(225, 180, 105));
        p.drawRect(QRectF(c.x() - 13, c.y() - 13, 26, 22));
        QPolygonF roof;
        roof << QPointF(c.x() - 17, c.y() - 13) << QPointF(c.x(), c.y() - 27)
             << QPointF(c.x() + 17, c.y() - 13);
        p.setBrush(QColor(174, 73, 62));
        p.drawPolygon(roof);
        p.setBrush(QColor(91, 139, 198));
        p.drawRect(QRectF(c.x() - 5, c.y() - 7, 10, 10));
    };
    drawHouse(-54.0, mLeftHouseAlive);
    drawHouse(54.0, mRightHouseAlive);

    const QPointF cannon = screen(cannonPosition());
    if (mCannonAlive) {
        const double heat = qBound(0.0, mHeatLevel / 100.0, 1.0);
        const double glow = qBound(0.0, (heat - 0.55) / 0.45, 1.0);
        const QColor baseColor = blendedColor(QColor(76, 87, 100), QColor(210, 48, 32), heat);
        const QColor barrelColor = blendedColor(QColor(125, 139, 151), QColor(255, 78, 38), heat);
        const QColor edgeColor = blendedColor(QColor(185, 195, 205), QColor(255, 182, 92), heat);

        if (glow > 0.0) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 48, 20, int(85.0 * glow)));
            p.drawEllipse(cannon + QPointF(0.0, -9.0), 21.0 + 7.0 * glow, 17.0 + 7.0 * glow);
        }

        p.setPen(QPen(edgeColor, 2));
        p.setBrush(baseColor.darker(125));
        QPolygonF carriage;
        carriage << cannon + QPointF(-18, 7) << cannon + QPointF(-12, -3)
                 << cannon + QPointF(12, -3) << cannon + QPointF(18, 7);
        p.drawPolygon(carriage);
        p.drawRoundedRect(QRectF(cannon.x() - 22, cannon.y() + 6, 44, 6), 2, 2);

        p.setBrush(baseColor);
        p.drawEllipse(cannon, 12, 10);
        p.setBrush(barrelColor.darker(120));
        p.drawEllipse(cannon, 5, 5);

        p.save();
        p.translate(cannon);
        p.rotate(mCurrentAngle);
        p.setPen(QPen(edgeColor, 2));
        p.setBrush(baseColor);
        QPolygonF shield;
        shield << QPointF(-11, -5) << QPointF(-9, -17)
               << QPointF(9, -17) << QPointF(11, -5);
        p.drawPolygon(shield);

        p.setBrush(barrelColor);
        p.drawRoundedRect(QRectF(-3.5, -35, 7, 34), 2, 2);
        p.drawRect(QRectF(-6, -38, 12, 5));
        p.setBrush(edgeColor);
        p.drawEllipse(QPointF(0, -1), 4, 4);
        p.restore();
    } else {
        p.setPen(QPen(QColor(225, 85, 65), 4));
        p.drawLine(cannon + QPointF(-14, -10), cannon + QPointF(14, 10));
        p.drawLine(cannon + QPointF(-14, 10), cannon + QPointF(14, -10));
    }

    for (const auto &asteroid : mAsteroids) {
        const QPointF c = screen(asteroid.position);
        const double r = qMax(5.0, asteroid.radius * scale);
        p.save();
        p.translate(c);
        p.rotate(asteroid.rotation);
        QPainterPath shape;
        shape.moveTo(0, -r);
        shape.lineTo(r * 0.75, -r * 0.45);
        shape.lineTo(r, r * 0.35);
        shape.lineTo(r * 0.25, r);
        shape.lineTo(-r * 0.75, r * 0.65);
        shape.lineTo(-r, -r * 0.25);
        shape.closeSubpath();
        p.setPen(QPen(QColor(181, 133, 94), 2));
        p.setBrush(QColor(105, 76, 61));
        p.drawPath(shape);
        p.setBrush(QColor(70, 52, 46));
        p.drawEllipse(QPointF(-r * 0.28, -r * 0.15), r * 0.2, r * 0.15);
        p.restore();
    }

    static const QColor particleColors[] = {
        QColor(126, 82, 50), QColor(169, 105, 54),
        QColor(213, 142, 65), QColor(239, 184, 91)
    };
    p.setPen(Qt::NoPen);
    for (const auto &particle : mHitParticles) {
        QColor color = particleColors[particle.colorIndex];
        color.setAlphaF(qBound(0.0, particle.life / particle.maxLife, 1.0));
        p.setBrush(color);
        const QPointF c = screen(particle.position);
        const double size = qMax(2.0, particle.size * scale * 0.55);
        p.drawRect(QRectF(c.x() - size * 0.5, c.y() - size * 0.5, size, size));
    }

    p.setPen(QPen(QColor(255, 224, 105), 2));
    for (const auto &projectile : mProjectiles) {
        const QPointF c = screen(projectile.position);
        p.drawLine(c, c + QPointF(-projectile.velocity.x(), projectile.velocity.y()) * 0.045);
    }
    p.restore();

    p.setPen(QColor(238, 241, 248));
    p.drawText(22, 29, QStringLiteral("Asteroid Defense  T+ %1 s").arg(mMissionTime, 0, 'f', 1));
    p.drawText(22, 49, QStringLiteral("Canon angle: %1 deg").arg(mCurrentAngle, 0, 'f', 1));

    const QRectF heatBar(field.right() - 174.0, field.top() + 15.0, 150.0, 14.0);
    p.setPen(QPen(QColor(205, 215, 225), 1));
    p.setBrush(QColor(40, 45, 55));
    p.drawRoundedRect(heatBar, 3, 3);
    const QColor heatColor = mHeatLevel < 65.0 ? QColor(75, 190, 110)
                                               : (mHeatLevel < 90.0 ? QColor(245, 180, 60) : QColor(235, 70, 58));
    p.setBrush(heatColor);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(QRectF(heatBar.left() + 2.0, heatBar.top() + 2.0,
                             (heatBar.width() - 4.0) * mHeatLevel / 100.0, heatBar.height() - 4.0), 2, 2);
    p.setPen(QColor(238, 241, 248));
    p.drawText(QRectF(heatBar.left(), heatBar.bottom() + 2.0, heatBar.width(), 18.0),
               Qt::AlignHCenter | Qt::AlignTop, QStringLiteral("Heat %1%").arg(mHeatLevel, 0, 'f', 0));

    if (isFinished()) {
        p.setPen(QColor(255, 110, 92));
        QFont font = p.font();
        font.setPointSize(font.pointSize() + 8);
        font.setBold(true);
        p.setFont(font);
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("SIMULATION OVER"));
    }
}

AsteroidDefenseScenario::AsteroidDefenseScenario()
    : mWidget(new AsteroidDefenseWidget())
    , mTimer(new QTimer(mWidget))
    , mRuntime(nullptr)
    , mRunAccumulator(0.0)
{
    mTimer->setInterval(40);
    QObject::connect(mTimer, &QTimer::timeout, [this]() { tick(); });
}

AsteroidDefenseScenario::~AsteroidDefenseScenario()
{
    delete mWidget;
}

QString AsteroidDefenseScenario::packageName() const
{
    return QStringLiteral("App.NeoAdaEdit.AsteroidDefense");
}

QString AsteroidDefenseScenario::title() const
{
    return QStringLiteral("Asteroid Defense");
}

QString AsteroidDefenseScenario::description() const
{
    return QStringLiteral("Schuetze Erde und Haeuser mit einer programmierbaren Abwehrkanone.");
}

QString AsteroidDefenseScenario::initialSource() const
{
    return QString::fromUtf8(R"SCN(with App.NeoAdaEdit.AsteroidDefense;

procedure onStart() is
begin
    Canon:fireStart();
end;

procedure run() is
begin
    if Canon:heatLevel() >= 85.0 then
        Canon:fireStop();
    elsif Canon:heatLevel() <= 45.0 then
        Canon:fireStart();
    end if;
end;

procedure onAsteroidDetected(id : Natural; x, y, dx, dy : Number) is
begin
    -- Dieses Ereignis kommt genau einmal pro neuem Asteroiden.
    -- Die ID kann fuer eine eigene Asteroiden-Liste verwendet werden.
    Canon:setTargetAngle(x);
end;

procedure onAsteroidDestroyed(id : Natural) is
begin
    -- Den Asteroiden mit dieser ID aus der eigenen Liste entfernen.
end;

return "Defense ready";
)SCN");
}

QWidget *AsteroidDefenseScenario::widget()
{
    return mWidget;
}

void AsteroidDefenseScenario::bind(NdaRuntime &runtime)
{
    mRuntime = &runtime;
    NdaState *state = runtime.state();
    state->bindPrc("Canon", "fireStart", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->fireStart();
        return true;
    });
    state->bindPrc("Canon", "fireStop", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->fireStop();
        return true;
    });
    state->bindPrc("Canon", "setTargetAngle", {{"angle", "Number", Nda::InMode}}, [this](const Nda::FncValues &args) -> bool {
        mWidget->setTargetAngle(args.at("angle").toDouble());
        return true;
    });
    state->bindFnc("Canon", "currentAngle", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNumber(state->numberType(), mWidget->currentAngle());
        return true;
    });
    state->bindFnc("Canon", "heatLevel", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNumber(state->numberType(), mWidget->heatLevel());
        return true;
    });
}

void AsteroidDefenseScenario::reset()
{
    stop();
    mWidget->reset();
    mRuntime = nullptr;
    mRunAccumulator = 0.0;
}

void AsteroidDefenseScenario::afterRun(NdaRuntime &runtime)
{
    mRuntime = &runtime;
    NdaVariants args;
    if (mRuntime->state()->hasFunction("", "onStart", args)) {
        mRuntime->invokePrc("onStart");
        if (mRuntime->hasError() || !mRuntime->state()->unhandledException().empty()) {
            showRuntimeError();
            return;
        }
    }
    mRunAccumulator = 0.0;
    mWidget->start();
    mTimer->start();
}

void AsteroidDefenseScenario::stop()
{
    mTimer->stop();
    mWidget->stop();
}

void AsteroidDefenseScenario::tick()
{
    if (!mRuntime || !mWidget->isVisible() || !mWidget->isRunning()) {
        mTimer->stop();
        return;
    }

    mWidget->simulationStep();

    mRunAccumulator += kDt;
    if (mRunAccumulator >= 0.1) {
        mRunAccumulator -= 0.1;
        NdaVariants runArgs;
        if (mRuntime->state()->hasFunction("", "run", runArgs)) {
            mRuntime->invokePrc("run");
            if (mRuntime->hasError() || !mRuntime->state()->unhandledException().empty()) {
                showRuntimeError();
                return;
            }
        }
    }

    const auto spawned = mWidget->takeSpawnedAsteroids();
    for (const auto &detection : spawned) {
        NdaVariants args;
        args.push_back(mRuntime->state()->toVariant(NdaValue(int64_t(detection.id))));
        args.push_back(mRuntime->state()->toVariant(NdaValue(detection.x)));
        args.push_back(mRuntime->state()->toVariant(NdaValue(detection.y)));
        args.push_back(mRuntime->state()->toVariant(NdaValue(detection.dx)));
        args.push_back(mRuntime->state()->toVariant(NdaValue(detection.dy)));
        if (mRuntime->state()->hasFunction("", "onAsteroidDetected", args)) {
            mRuntime->invokePrc("onAsteroidDetected", NdaValue(int64_t(detection.id)),
                                NdaValue(detection.x), NdaValue(detection.y),
                                NdaValue(detection.dx), NdaValue(detection.dy));
            if (mRuntime->hasError() || !mRuntime->state()->unhandledException().empty()) {
                showRuntimeError();
                return;
            }
        }
    }

    const auto destroyed = mWidget->takeDestroyedAsteroids();
    for (qint64 id : destroyed) {
        NdaVariants args;
        args.push_back(mRuntime->state()->toVariant(NdaValue(int64_t(id))));
        if (mRuntime->state()->hasFunction("", "onAsteroidDestroyed", args)) {
            mRuntime->invokePrc("onAsteroidDestroyed", NdaValue(int64_t(id)));
            if (mRuntime->hasError() || !mRuntime->state()->unhandledException().empty()) {
                showRuntimeError();
                return;
            }
        }
    }

    if (mWidget->isFinished())
        mTimer->stop();
}

void AsteroidDefenseScenario::showRuntimeError()
{
    const QString message = mRuntime->hasError()
            ? QString::fromStdString(mRuntime->lastError())
            : QStringLiteral("Unhandled exception: %1").arg(QString::fromStdString(mRuntime->state()->unhandledException()));
    stop();
    QMessageBox::critical(mWidget, QStringLiteral("NeoAda Exception"), message);
}

#include "rocketscenario.h"

#include <QtMath>
#include <QMessageBox>
#include <QWheelEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QTimer>

#include <runtime.h>
#include <state.h>

namespace {
constexpr double kPlanetRadius = 1020.0;
constexpr double kAtmosphereRadius = 100.0;
constexpr double kDt = 0.04;
}

//-------------------------------------------------------------------------------------------------
RocketWidget::RocketWidget(QWidget *parent)
    : QWidget(parent)
    , mX(0.0)
    , mY(kPlanetRadius + 6.0)
    , mVx(0.0)
    , mVy(0.0)
    , mAngle(0.0)
    , mStage(1)
    , mRunning(false)
    , mFuel(100.0)
    , mFlamePhase(0.0)
    , mMissionTime(0.0)
    , mLaunchZoom(1.0)
    , mOrbitZoom(1.0)
{
    setMinimumSize(720, 390);
    setAutoFillBackground(true);
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::reset()
{
    mX = 0.0;
    mY = kPlanetRadius + 6.0;
    mVx = 0.0;
    mVy = 0.0;
    mAngle = 0.0;
    mStage = 1;
    mRunning = false;
    mFuel = 100.0;
    mFlamePhase = 0.0;
    mMissionTime = 0.0;
    mTrajectory.clear();
    mTrajectory.push_back(QPointF(mX, mY));
    update();
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::start()
{
    mRunning = true;
    update();
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::stop()
{
    mRunning = false;
    update();
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::steerLeft()
{
    mAngle -= 2.0;
    if (mAngle < -55.0)
        mAngle = -55.0;
    update();
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::steerRight()
{
    mAngle += 2.0;
    if (mAngle > 55.0)
        mAngle = 55.0;
    update();
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::doStage()
{
    if (mStage < 3)
        ++mStage;
    update();
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::simulationStep()
{
    if (!mRunning)
        return;

    const double distance = qSqrt(mX * mX + mY * mY);
    const double gravity = 2400000.0 / qMax(distance * distance, 1.0);
    const double gx = -gravity * mX / qMax(distance, 1.0);
    const double gy = -gravity * mY / qMax(distance, 1.0);

    const double requestedThrust = mStage == 1 ? 5.2 : (mStage == 2 ? 3.7 : 2.1);
    const double burnRate = mStage == 1 ? 0.22 : (mStage == 2 ? 0.14 : 0.08);
    const bool hasFuel = mFuel > 0.0;
    const double thrust = hasFuel ? requestedThrust : 0.0;
    const double angleRad = qDegreesToRadians(mAngle);
    const double tx = qSin(angleRad) * thrust;
    const double ty = qCos(angleRad) * thrust;

    if (hasFuel) {
        mFuel -= burnRate;
        if (mFuel < 0.0)
            mFuel = 0.0;
    }

    mMissionTime += kDt;
    mVx += (gx + tx) * kDt;
    mVy += (gy + ty) * kDt;
    mX += mVx * kDt;
    mY += mVy * kDt;
    mFlamePhase += 0.35;
    mTrajectory.push_back(QPointF(mX, mY));
    if (mTrajectory.size() > 1200)
        mTrajectory.remove(0, mTrajectory.size() - 1200);

    if (distance < kPlanetRadius + 2.0 && mVy < 0.0) {
        mY = kPlanetRadius + 2.0;
        mVy = 0.0;
        mVx *= 0.5;
        mRunning = false;
    }

    update();
}

//-------------------------------------------------------------------------------------------------
bool RocketWidget::isRunning() const
{
    return mRunning;
}

//-------------------------------------------------------------------------------------------------
double RocketWidget::x() const
{
    return mX;
}

//-------------------------------------------------------------------------------------------------
double RocketWidget::y() const
{
    return mY;
}

//-------------------------------------------------------------------------------------------------
double RocketWidget::height() const
{
    return qMax(0.0, qSqrt(mX * mX + mY * mY) - kPlanetRadius);
}

//-------------------------------------------------------------------------------------------------
double RocketWidget::fuel() const
{
    return mFuel;
}

//-------------------------------------------------------------------------------------------------
double RocketWidget::speed() const
{
    return qSqrt(mVx * mVx + mVy * mVy);
}

//-------------------------------------------------------------------------------------------------
double RocketWidget::missionTime() const
{
    return mMissionTime;
}

//-------------------------------------------------------------------------------------------------
int RocketWidget::state() const
{
    return mStage;
}

//-------------------------------------------------------------------------------------------------
QPointF RocketWidget::worldToScreen(double x, double y) const
{
    const QPointF center(width() * 0.34, height() * 0.78);
    return center + QPointF(x, -y);
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::drawPlanet(QPainter &p) const
{
    const QPointF planetCenter = worldToScreen(0.0, 0.0);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(72, 126, 208, 70));
    p.drawEllipse(planetCenter, kAtmosphereRadius, kAtmosphereRadius);

    p.setBrush(QColor(42, 96, 180));
    p.drawEllipse(planetCenter, kPlanetRadius, kPlanetRadius);

    p.setBrush(QColor(68, 150, 92));
    p.drawEllipse(planetCenter + QPointF(-70, -36), 42, 24);
    p.drawEllipse(planetCenter + QPointF(58, 18), 52, 28);
    p.drawEllipse(planetCenter + QPointF(6, -82), 34, 18);

    p.setPen(QPen(QColor(180, 220, 255, 120), 2));
    p.drawEllipse(planetCenter, kPlanetRadius + 8, kPlanetRadius + 8);

    p.setPen(QPen(QColor(210, 220, 235, 120), 1, Qt::DashLine));
    p.drawEllipse(planetCenter, kPlanetRadius + 95, kPlanetRadius + 95);
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::drawRocket(QPainter &p) const
{
    const QPointF rocketCenter = worldToScreen(mX, mY);

    p.save();
    p.translate(rocketCenter);
    p.rotate(mAngle);

    p.setPen(QPen(QColor(210, 210, 220), 1));
    p.setBrush(QColor(236, 238, 244));
    p.drawRoundedRect(QRectF(-9, -34, 18, 50), 7, 7);

    QPolygonF nose;
    nose << QPointF(0, -48) << QPointF(10, -31) << QPointF(-10, -31);
    p.setBrush(QColor(215, 68, 68));
    p.drawPolygon(nose);

    p.setBrush(QColor(82, 142, 218));
    p.drawEllipse(QPointF(0, -18), 5, 5);

    p.setBrush(QColor(70, 88, 110));
    QPolygonF leftFin;
    leftFin << QPointF(-9, 4) << QPointF(-20, 18) << QPointF(-8, 14);
    QPolygonF rightFin;
    rightFin << QPointF(9, 4) << QPointF(20, 18) << QPointF(8, 14);
    p.drawPolygon(leftFin);
    p.drawPolygon(rightFin);

    if (mRunning) {
        const double flame = 14.0 + 5.0 * qSin(mFlamePhase);
        QPolygonF fire;
        fire << QPointF(-7, 15) << QPointF(0, 15 + flame) << QPointF(7, 15);
        p.setBrush(QColor(255, 132, 35));
        p.setPen(Qt::NoPen);
        p.drawPolygon(fire);
        p.setBrush(QColor(255, 230, 84));
        p.drawEllipse(QPointF(0, 22), 4, flame * 0.35);
    }

    p.restore();
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(12, 15, 26));

    const QRect launchRect = rect().adjusted(10, 10, -(width() / 2 + 5), -10);
    const QRect orbitRect = rect().adjusted(width() / 2 + 5, 10, -10, -10);

    p.setPen(QPen(QColor(58, 64, 82), 1));
    p.setBrush(QColor(18, 23, 38));
    p.drawRoundedRect(launchRect, 6, 6);
    p.drawRoundedRect(orbitRect, 6, 6);

    p.setPen(QColor(230, 235, 245));
    for (int i = 0; i < 90; ++i) {
        const int x = orbitRect.left() + (i * 73 + 19) % qMax(1, orbitRect.width());
        const int y = orbitRect.top() + (i * 41 + 29) % qMax(1, orbitRect.height());
        const int alpha = 90 + (i * 37) % 150;
        p.setPen(QColor(225, 235, 255, alpha));
        p.drawPoint(x, y);
        if (i % 17 == 0)
            p.drawPoint(x + 1, y);
    }

    p.setPen(QColor(230, 235, 245));
    p.drawText(launchRect.adjusted(10, 8, -10, -8), Qt::AlignTop | Qt::AlignLeft, QStringLiteral("2D Launch View"));
    p.drawText(orbitRect.adjusted(10, 8, -10, -8), Qt::AlignTop | Qt::AlignLeft, QStringLiteral("Orbital View"));

    auto drawSmallRocket = [&](const QPointF &center, double angle, double scale, bool flame) {
        p.save();
        p.translate(center);
        p.scale(scale, scale);
        p.rotate(angle);
        p.setPen(QPen(QColor(210, 210, 220), 1));
        p.setBrush(QColor(236, 238, 244));
        p.drawRoundedRect(QRectF(-9, -34, 18, 50), 7, 7);
        QPolygonF nose;
        nose << QPointF(0, -48) << QPointF(10, -31) << QPointF(-10, -31);
        p.setBrush(QColor(215, 68, 68));
        p.drawPolygon(nose);
        p.setBrush(QColor(82, 142, 218));
        p.drawEllipse(QPointF(0, -18), 5, 5);
        p.setBrush(QColor(70, 88, 110));
        QPolygonF leftFin;
        leftFin << QPointF(-9, 4) << QPointF(-20, 18) << QPointF(-8, 14);
        QPolygonF rightFin;
        rightFin << QPointF(9, 4) << QPointF(20, 18) << QPointF(8, 14);
        p.drawPolygon(leftFin);
        p.drawPolygon(rightFin);
        if (flame) {
            const double flameLen = 14.0 + 5.0 * qSin(mFlamePhase);
            QPolygonF fire;
            fire << QPointF(-7, 15) << QPointF(0, 15 + flameLen) << QPointF(7, 15);
            p.setBrush(QColor(255, 132, 35));
            p.setPen(Qt::NoPen);
            p.drawPolygon(fire);
            p.setBrush(QColor(255, 230, 84));
            p.drawEllipse(QPointF(0, 22), 4, flameLen * 0.35);
        }
        p.restore();
    };

    // Left: local 2D launch view. Camera follows the rocket; down is always Earth.
    p.save();
    p.setClipRect(launchRect.adjusted(1, 1, -1, -1));
    const double localHeight = height();
    const double localScale = 1.05 * mLaunchZoom;
    const double horizontalScale = 0.34 * mLaunchZoom;
    const QPointF localRocket(launchRect.center().x(), launchRect.center().y() + 18.0);
    const double launchGround = localRocket.y() + localHeight * localScale + 22.0;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(75, 116, 70));
    p.drawRect(QRectF(launchRect.left(), launchGround, launchRect.width(), launchRect.bottom() - launchGround));
    p.setBrush(QColor(65, 103, 178, 130));
    p.drawRect(QRectF(launchRect.left(), launchGround + 18, launchRect.width(), 20));

    p.setPen(QPen(QColor(150, 160, 170), 3));
    const QPointF pad(localRocket.x() - mX * horizontalScale, launchGround);
    p.drawLine(pad + QPointF(-34, 0), pad + QPointF(34, 0));
    p.drawLine(pad + QPointF(-24, 0), pad + QPointF(-4, -44));
    p.drawLine(pad + QPointF(24, 0), pad + QPointF(4, -44));
    p.setPen(QPen(QColor(180, 100, 64), 4));
    p.drawLine(pad + QPointF(-42, 0), pad + QPointF(42, 0));

    if (mTrajectory.size() > 1) {
        QPainterPath path;
        const auto localPoint = [&](const QPointF &pt) {
            const double h = qMax(0.0, qSqrt(pt.x() * pt.x() + pt.y() * pt.y()) - kPlanetRadius);
            return QPointF(localRocket.x() + (pt.x() - mX) * horizontalScale,
                           localRocket.y() - (h - localHeight) * localScale);
        };
        path.moveTo(localPoint(mTrajectory.first()));
        for (const auto &pt : mTrajectory)
            path.lineTo(localPoint(pt));
        p.setPen(QPen(QColor(255, 210, 95, 170), 2));
        p.drawPath(path);
    }

    drawSmallRocket(localRocket, mAngle, 0.82, mRunning && mFuel > 0.0);
    p.restore();

    // Right: orbital view around the planet, including the full trajectory.
    p.save();
    p.setClipRect(orbitRect.adjusted(1, 1, -1, -1));
    const QPointF orbitCenter(orbitRect.center().x(), orbitRect.center().y() + orbitRect.height() * 0.02);
    const double rocketDistance = qSqrt(mX * mX + mY * mY);
    const double viewRadius = qMax(kPlanetRadius * 2.18, rocketDistance * 1.18);
    const double orbitScale = qMin(orbitRect.width(), orbitRect.height()) / (viewRadius * 2.0) * 0.88 * mOrbitZoom;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(86, 154, 236, 58));
    p.drawEllipse(orbitCenter, kAtmosphereRadius * orbitScale, kAtmosphereRadius * orbitScale);

    QRadialGradient earthGradient(orbitCenter - QPointF(42, 58) * orbitScale, kPlanetRadius * orbitScale * 1.18);
    earthGradient.setColorAt(0.0, QColor(96, 177, 245));
    earthGradient.setColorAt(0.55, QColor(32, 103, 202));
    earthGradient.setColorAt(1.0, QColor(8, 39, 120));
    p.setBrush(earthGradient);
    p.drawEllipse(orbitCenter, kPlanetRadius * orbitScale, kPlanetRadius * orbitScale);

    p.setBrush(QColor(52, 166, 99, 215));
    p.drawEllipse(orbitCenter + QPointF(-118, -52) * orbitScale, 86 * orbitScale, 38 * orbitScale);
    p.drawEllipse(orbitCenter + QPointF(84, 36) * orbitScale, 105 * orbitScale, 44 * orbitScale);
    p.drawEllipse(orbitCenter + QPointF(14, -142) * orbitScale, 58 * orbitScale, 25 * orbitScale);
    p.setBrush(QColor(242, 247, 255, 125));
    p.drawEllipse(orbitCenter + QPointF(-54, -116) * orbitScale, 120 * orbitScale, 18 * orbitScale);
    p.drawEllipse(orbitCenter + QPointF(90, -44) * orbitScale, 86 * orbitScale, 14 * orbitScale);

    p.setPen(QPen(QColor(190, 225, 255, 150), 2));
    p.drawEllipse(orbitCenter, (kPlanetRadius + 10) * orbitScale, (kPlanetRadius + 10) * orbitScale);
    p.setPen(QPen(QColor(210, 220, 235, 120), 1, Qt::DashLine));
    p.drawEllipse(orbitCenter, (kPlanetRadius * 2.0) * orbitScale, (kPlanetRadius * 2.0) * orbitScale);

    auto orbitPoint = [&](const QPointF &world) {
        return orbitCenter + QPointF(world.x() * orbitScale, -world.y() * orbitScale);
    };
    if (mTrajectory.size() > 1) {
        QPainterPath path;
        path.moveTo(orbitPoint(mTrajectory.first()));
        for (const auto &pt : mTrajectory)
            path.lineTo(orbitPoint(pt));
        p.setPen(QPen(QColor(255, 210, 95, 190), 2));
        p.drawPath(path);
    }
    drawSmallRocket(orbitPoint(QPointF(mX, mY)), mAngle, 0.45, mRunning && mFuel > 0.0);
    p.restore();

    p.setPen(QColor(238, 240, 248));
    p.drawText(launchRect.adjusted(10, 28, -10, -8), Qt::AlignTop | Qt::AlignLeft,
               QStringLiteral("height=%1  v=%2  fuel=%3  stage=%4  %5  zoom=%6x").arg(height(), 0, 'f', 1).arg(speed(), 0, 'f', 2).arg(mFuel, 0, 'f', 1).arg(mStage).arg(mRunning ? QStringLiteral("running") : QStringLiteral("stopped")).arg(mLaunchZoom, 0, 'f', 1));
    p.drawText(launchRect.adjusted(10, -34, -10, -8), Qt::AlignBottom | Qt::AlignHCenter,
               QStringLiteral("T+ %1 s").arg(mMissionTime, 0, 'f', 1));
    p.drawText(orbitRect.adjusted(10, 28, -10, -8), Qt::AlignTop | Qt::AlignLeft,
               QStringLiteral("x=%1  y=%2  v=%3  zoom=%4x").arg(mX, 0, 'f', 1).arg(mY, 0, 'f', 1).arg(speed(), 0, 'f', 2).arg(mOrbitZoom, 0, 'f', 1));
}

//-------------------------------------------------------------------------------------------------
void RocketWidget::wheelEvent(QWheelEvent *event)
{
    const QRect launchRect = rect().adjusted(10, 10, -(width() / 2 + 5), -10);
    const QRect orbitRect = rect().adjusted(width() / 2 + 5, 10, -10, -10);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const QPoint pos = event->position().toPoint();
#else
    const QPoint pos = event->pos();
#endif
    const double factor = event->angleDelta().y() > 0 ? 1.12 : 1.0 / 1.12;

    if (launchRect.contains(pos)) {
        mLaunchZoom = qBound(0.35, mLaunchZoom * factor, 5.0);
        update();
        event->accept();
        return;
    }
    if (orbitRect.contains(pos)) {
        mOrbitZoom = qBound(0.35, mOrbitZoom * factor, 5.0);
        update();
        event->accept();
        return;
    }

    QWidget::wheelEvent(event);
}


//-------------------------------------------------------------------------------------------------
RocketScenario::RocketScenario()
    : mWidget(new RocketWidget())
    , mTimer(new QTimer(mWidget))
    , mRuntime(nullptr)
{
    mTimer->setInterval(40);
    QObject::connect(mTimer, &QTimer::timeout, [this]() {
        tick();
    });
}

//-------------------------------------------------------------------------------------------------
RocketScenario::~RocketScenario()
{
    delete mWidget;
}

//-------------------------------------------------------------------------------------------------
QString RocketScenario::packageName() const
{
    return QStringLiteral("App.NeoAdaEdit.Rocket");
}

//-------------------------------------------------------------------------------------------------
QString RocketScenario::title() const
{
    return QStringLiteral("Rocket Launch");
}

//-------------------------------------------------------------------------------------------------
QString RocketScenario::description() const
{
    return QStringLiteral("Programmiere eine Rakete in Richtung Umlaufbahn und lerne einfache Regelungstechnik.");
}

//-------------------------------------------------------------------------------------------------
QString RocketScenario::initialSource() const
{
    return QString::fromUtf8(R"SCN(with App.NeoAdaEdit.Rocket;

procedure onStart() is
begin
    Rocket:start();
end;

procedure run() is
begin
    if Rocket:fuel() > 0.0 then
        if Rocket:height() > 95.0 then
            Rocket:steerRight();
        elsif Rocket:height() > 45.0 then
            Rocket:steerLeft();
        end if;

        if Rocket:state() = 1 and Rocket:height() > 120.0 then
            Rocket:doStage();
        end if;
    end if;
end;

return "Rocket ready";
)SCN");
}

//-------------------------------------------------------------------------------------------------
QWidget *RocketScenario::widget()
{
    return mWidget;
}

//-------------------------------------------------------------------------------------------------
void RocketScenario::bind(NdaRuntime &runtime)
{
    mRuntime = &runtime;
    NdaState *state = runtime.state();
    state->bindPrc("Rocket", "start", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->start();
        return true;
    });
    state->bindPrc("Rocket", "stop", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->stop();
        return true;
    });
    state->bindPrc("Rocket", "steerLeft", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->steerLeft();
        return true;
    });
    state->bindPrc("Rocket", "steerRight", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->steerRight();
        return true;
    });
    state->bindPrc("Rocket", "doStage", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->doStage();
        return true;
    });
    state->bindFnc("Rocket", "state", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNatural(state->naturalType(), mWidget->state());
        return true;
    });
    state->bindFnc("Rocket", "x", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNumber(state->numberType(), mWidget->x());
        return true;
    });
    state->bindFnc("Rocket", "y", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNumber(state->numberType(), mWidget->y());
        return true;
    });
    state->bindFnc("Rocket", "height", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNumber(state->numberType(), mWidget->height());
        return true;
    });
    state->bindFnc("Rocket", "fuel", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNumber(state->numberType(), mWidget->fuel());
        return true;
    });
    state->bindFnc("Rocket", "v", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNumber(state->numberType(), mWidget->speed());
        return true;
    });
    state->bindFnc("Rocket", "missionTime", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNumber(state->numberType(), mWidget->missionTime());
        return true;
    });
}

//-------------------------------------------------------------------------------------------------
void RocketScenario::reset()
{
    stop();
    mWidget->reset();
    mRuntime = nullptr;
}

//-------------------------------------------------------------------------------------------------
void RocketScenario::afterRun(NdaRuntime &runtime)
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

    mTimer->start();
}

//-------------------------------------------------------------------------------------------------
void RocketScenario::stop()
{
    mTimer->stop();
    mWidget->stop();
}

//-------------------------------------------------------------------------------------------------
void RocketScenario::tick()
{
    if (!mRuntime || !mWidget->isVisible()) {
        mTimer->stop();
        return;
    }

    mWidget->simulationStep();

    NdaVariants args;
    if (mRuntime->state()->hasFunction("", "run", args)) {
        mRuntime->invokePrc("run");
        if (mRuntime->hasError() || !mRuntime->state()->unhandledException().empty()) {
            showRuntimeError();
            return;
        }
    }
}

//-------------------------------------------------------------------------------------------------
void RocketScenario::showRuntimeError()
{
    const QString message = mRuntime->hasError()
            ? QString::fromStdString(mRuntime->lastError())
            : QStringLiteral("Unhandled exception: %1").arg(QString::fromStdString(mRuntime->state()->unhandledException()));
    stop();
    QMessageBox::critical(mWidget, QStringLiteral("NeoAda Exception"), message);
}

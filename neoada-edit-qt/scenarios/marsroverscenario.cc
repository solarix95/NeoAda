#include "marsroverscenario.h"

#include <QtMath>
#include <QMessageBox>
#include <QPainter>
#include <QPolygonF>
#include <QPushButton>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QTimer>

#include <runtime.h>
#include <state.h>

MarsRoverWidget::MarsRoverWidget(QWidget *parent)
    : QWidget(parent)
    , mX(0.0)
    , mY(0.0)
    , mDirection(North)
    , mRunning(false)
    , mWheelPhase(0.0)
    , mRadarPhase(0.0)
    , mTargetX(4.0)
    , mTargetY(1.0)
    , mRandomButton(new QPushButton(QStringLiteral("Random"), this))
{
    setMinimumSize(360, 300);
    setAutoFillBackground(true);
    loadDefaultMap();
    connect(mRandomButton, &QPushButton::clicked, [this]() {
        randomizeMap();
    });
}

QRect MarsRoverWidget::playFieldRect() const
{
    return rect().adjusted(0, 0, 0, -44);
}

void MarsRoverWidget::loadDefaultMap()
{
    mRocks.clear();
    mRocks.push_back({3, -2, 0.42, 0.30});
    mRocks.push_back({-3, 2, 0.55, 0.36});
    mRocks.push_back({1.5, 2.2, 0.28, 0.18});
    mRocks.push_back({-1.8, -1.6, 0.32, 0.22});
    mTargetX = 4.0;
    mTargetY = 1.0;
}

void MarsRoverWidget::randomizeMap()
{
    stop();
    mX = 0.0;
    mY = 0.0;
    mDirection = North;
    mWheelPhase = 0.0;
    mRadarPhase = 0.0;

    mRocks.clear();
    auto *rng = QRandomGenerator::global();
    for (int i = 0; i < 9; ++i) {
        double x = 0.0;
        double y = 0.0;
        do {
            x = rng->bounded(-7, 8);
            y = rng->bounded(-5, 6);
        } while (qAbs(x) < 1.5 && qAbs(y) < 1.5);
        const double rx = 0.25 + rng->bounded(25) / 100.0;
        const double ry = 0.18 + rng->bounded(22) / 100.0;
        mRocks.push_back({x, y, rx, ry});
    }

    do {
        mTargetX = rng->bounded(-8, 9);
        mTargetY = rng->bounded(-6, 7);
    } while (qAbs(mTargetX) < 2.0 && qAbs(mTargetY) < 2.0);

    update();
}

void MarsRoverWidget::reset()
{
    mX = 0.0;
    mY = 0.0;
    mDirection = North;
    mRunning = false;
    mWheelPhase = 0.0;
    mRadarPhase = 0.0;
    update();
}

void MarsRoverWidget::start()
{
    mRunning = true;
    update();
}

void MarsRoverWidget::stop()
{
    mRunning = false;
    update();
}

void MarsRoverWidget::forward()
{
    constexpr double step = 0.08;
    mRadarPhase += 12.0;
    switch (mDirection) {
    case North: mY -= step; break;
    case East:  mX += step; break;
    case South: mY += step; break;
    case West:  mX -= step; break;
    }
    mWheelPhase += 22.0;
    update();
}

void MarsRoverWidget::backward()
{
    constexpr double step = 0.08;
    mRadarPhase += 12.0;
    switch (mDirection) {
    case North: mY += step; break;
    case East:  mX -= step; break;
    case South: mY -= step; break;
    case West:  mX += step; break;
    }
    mWheelPhase -= 22.0;
    update();
}

void MarsRoverWidget::steerLeft()
{
    mDirection = static_cast<Direction>((mDirection + 3) % 4);
    mRadarPhase += 12.0;
    update();
}

void MarsRoverWidget::steerRight()
{
    mDirection = static_cast<Direction>((mDirection + 1) % 4);
    mRadarPhase += 12.0;
    update();
}

void MarsRoverWidget::animateRadar()
{
    if (!mRunning)
        return;
    mRadarPhase += 8.0;
    if (mRadarPhase >= 360.0)
        mRadarPhase -= 360.0;
    update();
}

bool MarsRoverWidget::isRunning() const
{
    return mRunning;
}

double MarsRoverWidget::x() const
{
    return mX;
}

double MarsRoverWidget::y() const
{
    return mY;
}

QString MarsRoverWidget::sensorValue() const
{
    const double lookAhead = 0.65;
    const double probeX = mX + (mDirection == East ? lookAhead : (mDirection == West ? -lookAhead : 0.0));
    const double probeY = mY + (mDirection == South ? lookAhead : (mDirection == North ? -lookAhead : 0.0));
    const int nextX = qRound(probeX);
    const int nextY = qRound(probeY);

    for (const auto &rock : mRocks) {
        const double dx = probeX - rock.x;
        const double dy = probeY - rock.y;
        if ((dx * dx) / (rock.rx * rock.rx) + (dy * dy) / (rock.ry * rock.ry) <= 1.0)
            return QStringLiteral("blocked");
    }
    if (qAbs(probeX - mTargetX) < 0.55 && qAbs(probeY - mTargetY) < 0.55)
        return QStringLiteral("target");
    if ((nextX + nextY) % 4 == 0)
        return QStringLiteral("rough");
    return QStringLiteral("free");
}

void MarsRoverWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRect field = playFieldRect();
    p.fillRect(rect(), QColor(46, 42, 40));
    p.fillRect(field, QColor(73, 53, 44));
    p.setClipRect(field);

    constexpr int grid = 36;
    const QPointF viewCenter(field.center());
    const QPointF camera = viewCenter - QPointF(mX * grid, mY * grid);

    auto worldPoint = [&](double x, double y) -> QPointF {
        return camera + QPointF(x * grid, y * grid);
    };

    const int minGridX = qFloor((0.0 - camera.x()) / grid) - 1;
    const int maxGridX = qCeil((width() - camera.x()) / grid) + 1;
    const int minGridY = qFloor((0.0 - camera.y()) / grid) - 1;
    const int maxGridY = qCeil((height() - camera.y()) / grid) + 1;

    p.setPen(QPen(QColor(105, 75, 62), 1));
    for (int gx = minGridX; gx <= maxGridX; ++gx) {
        const QPointF a = worldPoint(gx, minGridY);
        const QPointF b = worldPoint(gx, maxGridY);
        p.drawLine(QPointF(a.x(), 0), QPointF(b.x(), height()));
    }
    for (int gy = minGridY; gy <= maxGridY; ++gy) {
        const QPointF a = worldPoint(minGridX, gy);
        const QPointF b = worldPoint(maxGridX, gy);
        p.drawLine(QPointF(0, a.y()), QPointF(width(), b.y()));
    }

    p.setPen(QPen(QColor(172, 111, 78), 2));
    p.setBrush(QColor(126, 78, 57));
    for (const auto &rock : mRocks) {
        const QPointF c = worldPoint(rock.x, rock.y);
        p.drawEllipse(c, rock.rx * grid, rock.ry * grid);
    }

    const QPointF target = worldPoint(mTargetX, mTargetY);
    p.setBrush(QColor(93, 178, 88));
    p.setPen(QPen(QColor(190, 238, 146), 3));
    p.drawEllipse(target, 0.36 * grid, 0.36 * grid);
    p.setPen(QPen(QColor(236, 255, 206), 2));
    p.drawLine(target + QPointF(-9, 0), target + QPointF(9, 0));
    p.drawLine(target + QPointF(0, -9), target + QPointF(0, 9));

    const QPointF roverCenter = viewCenter;
    p.save();
    p.translate(roverCenter);
    p.rotate(mDirection * 90);

    auto drawWheel = [&](const QPointF &center) {
        p.save();
        p.translate(center);
        p.rotate(mWheelPhase);
        p.setPen(QPen(QColor(42, 39, 42), 3));
        p.setBrush(QColor(67, 62, 66));
        p.drawEllipse(QPointF(0, 0), 6.5, 9.0);
        p.setPen(QPen(QColor(190, 178, 150), 1.5));
        p.drawLine(QPointF(-5, 0), QPointF(5, 0));
        p.drawLine(QPointF(0, -7), QPointF(0, 7));
        p.restore();
    };

    p.setPen(QPen(QColor(96, 72, 58), 3));
    p.drawLine(QPointF(-18, -12), QPointF(-18, 13));
    p.drawLine(QPointF(18, -12), QPointF(18, 13));
    drawWheel(QPointF(-22, -12));
    drawWheel(QPointF(-22, 12));
    drawWheel(QPointF(22, -12));
    drawWheel(QPointF(22, 12));

    p.setPen(QPen(QColor(255, 236, 176), 2));
    p.setBrush(mRunning ? QColor(255, 198, 76) : QColor(214, 204, 184));
    p.drawRoundedRect(QRectF(-17, -18, 34, 36), 8, 8);

    p.setBrush(QColor(45, 93, 150));
    p.setPen(QPen(QColor(165, 220, 255), 1));
    p.drawRoundedRect(QRectF(-13, 3, 26, 11), 3, 3);
    p.setPen(QPen(QColor(116, 178, 220), 1));
    p.drawLine(QPointF(-4, 4), QPointF(-4, 13));
    p.drawLine(QPointF(5, 4), QPointF(5, 13));
    p.drawLine(QPointF(-12, 8), QPointF(12, 8));

    p.setBrush(QColor(116, 190, 255));
    p.setPen(QPen(QColor(240, 250, 255), 1));
    p.drawRoundedRect(QRectF(-9, -10, 18, 7), 2, 2);

    p.save();
    p.translate(QPointF(0, -7));
    p.rotate(mRadarPhase);
    p.setPen(QPen(QColor(235, 245, 255), 2));
    p.drawLine(QPointF(0, 0), QPointF(17, 0));
    p.setPen(QPen(QColor(103, 170, 235), 2));
    p.drawArc(QRectF(9, -7, 14, 14), -55 * 16, 110 * 16);
    p.setBrush(QColor(105, 180, 245));
    p.setPen(QPen(QColor(230, 245, 255), 1));
    p.drawEllipse(QPointF(0, 0), 3, 3);
    p.restore();

    QPolygonF cabin;
    cabin << QPointF(0, -27) << QPointF(10, -17) << QPointF(-10, -17);
    p.setBrush(QColor(104, 154, 230));
    p.setPen(QPen(QColor(232, 242, 255), 1));
    p.drawPolygon(cabin);

    p.setPen(QPen(QColor(255, 222, 120), 2.5));
    p.drawLine(QPointF(8, -18), QPointF(18, -31));
    p.setBrush(QColor(255, 91, 91));
    p.setPen(QPen(QColor(255, 220, 220), 1));
    p.drawEllipse(QPointF(20, -34), 3.8, 3.8);

    p.setPen(QPen(QColor(255, 245, 180), 2));
    p.drawLine(QPointF(-13, -20), QPointF(-25, -28));
    p.setBrush(QColor(40, 95, 160));
    p.setPen(QPen(QColor(170, 225, 255), 1));
    p.drawRoundedRect(QRectF(-39, -34, 16, 10), 2, 2);
    p.drawLine(QPointF(-34, -33), QPointF(-34, -25));
    p.drawLine(QPointF(-28, -33), QPointF(-28, -25));
    p.restore();

    p.setPen(QColor(245, 238, 220));
    p.drawText(12, 22, QStringLiteral("MarsRover (%1,%2)").arg(mX, 0, 'f', 1).arg(mY, 0, 'f', 1));
    p.drawText(12, 42, mRunning ? QStringLiteral("running") : QStringLiteral("stopped"));
    p.drawText(12, 62, QStringLiteral("sensor: %1").arg(sensorValue()));
    p.setClipping(false);
    p.setPen(QPen(QColor(88, 82, 78), 1));
    p.drawLine(0, field.bottom() + 1, width(), field.bottom() + 1);
}

void MarsRoverWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    const int w = 92;
    const int h = 28;
    mRandomButton->setGeometry((width() - w) / 2, height() - h - 8, w, h);
}


MarsRoverScenario::MarsRoverScenario()
    : mWidget(new MarsRoverWidget())
    , mTimer(new QTimer(mWidget))
    , mRuntime(nullptr)
{
    mTimer->setInterval(40);
    QObject::connect(mTimer, &QTimer::timeout, [this]() {
        tick();
    });
}

MarsRoverScenario::~MarsRoverScenario()
{
    delete mWidget;
}

QString MarsRoverScenario::packageName() const
{
    return QStringLiteral("App.NeoAdaEdit.MarsRover");
}

QString MarsRoverScenario::title() const
{
    return QStringLiteral("MarsRover");
}

QString MarsRoverScenario::description() const
{
    return QStringLiteral("Steuere einen kleinen Rover und lerne Events, Sensoren und Zustandslogik.");
}

QString MarsRoverScenario::initialSource() const
{
    return QString::fromUtf8(R"SCN(with App.NeoAdaEdit.MarsRover;

procedure onStart() is
begin
    Rover:start();
end;

procedure onMove(sensor : String) is
begin
    if sensor = "blocked" then
        Rover:steerRight();
    elsif sensor = "target" then
        Rover:stop();
    else
        Rover:forward();
    end if;
end;

return "MarsRover bereit";
)SCN");
}

QWidget *MarsRoverScenario::widget()
{
    return mWidget;
}

void MarsRoverScenario::bind(NdaRuntime &runtime)
{
    mRuntime = &runtime;
    NdaState *state = runtime.state();
    state->bindPrc("Rover", "start", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->start();
        return true;
    });
    state->bindPrc("Rover", "stop", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->stop();
        return true;
    });
    state->bindPrc("Rover", "forward", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->forward();
        return true;
    });
    state->bindPrc("Rover", "backward", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->backward();
        return true;
    });
    state->bindPrc("Rover", "steerLeft", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->steerLeft();
        return true;
    });
    state->bindPrc("Rover", "steerRight", {}, [this](const Nda::FncValues &) -> bool {
        mWidget->steerRight();
        return true;
    });
    state->bindFnc("Rover", "sensor", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromString(state->stringType(), mWidget->sensorValue().toStdString());
        return true;
    });
    state->bindFnc("Rover", "x", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNumber(state->numberType(), mWidget->x());
        return true;
    });
    state->bindFnc("Rover", "y", {}, [state, this](const Nda::FncValues &, NdaVariant &ret) -> bool {
        ret.fromNumber(state->numberType(), mWidget->y());
        return true;
    });
}

void MarsRoverScenario::reset()
{
    stop();
    mWidget->reset();
    mRuntime = nullptr;
}

void MarsRoverScenario::afterRun(NdaRuntime &runtime)
{
    mRuntime = &runtime;

    NdaVariants args;
    if (mRuntime->state()->hasFunction("", "onStart", args)) {
        mRuntime->invokePrc("onStart");
        if (mRuntime->hasError() || !mRuntime->state()->unhandledException().empty()) {
            const QString message = mRuntime->hasError()
                    ? QString::fromStdString(mRuntime->lastError())
                    : QStringLiteral("Unhandled exception: %1").arg(QString::fromStdString(mRuntime->state()->unhandledException()));
            stop();
            QMessageBox::critical(mWidget, QStringLiteral("NeoAda Exception"), message);
            return;
        }
    }

    if (mWidget->isRunning())
        mTimer->start();
}

void MarsRoverScenario::stop()
{
    mTimer->stop();
    mWidget->stop();
}

void MarsRoverScenario::tick()
{
    if (!mRuntime || !mWidget->isVisible() || !mWidget->isRunning()) {
        mTimer->stop();
        return;
    }

    mWidget->animateRadar();

    NdaVariants args;
    args.push_back(mRuntime->state()->toVariant(NdaValue(mWidget->sensorValue().toStdString())));
    if (!mRuntime->state()->hasFunction("", "onMove", args))
        return;

    mRuntime->invokePrc("onMove", NdaValue(mWidget->sensorValue().toStdString()));
    if (mRuntime->hasError() || !mRuntime->state()->unhandledException().empty()) {
        const QString message = mRuntime->hasError()
                ? QString::fromStdString(mRuntime->lastError())
                : QStringLiteral("Unhandled exception: %1").arg(QString::fromStdString(mRuntime->state()->unhandledException()));
        stop();
        QMessageBox::critical(mWidget, QStringLiteral("NeoAda Exception"), message);
    }
}

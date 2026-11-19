#include "ui/AnalogClockWidget.hpp"
#include "core/TimeEngine.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QWheelEvent>

#include <cmath>

namespace qworldclock {

AnalogClockWidget::AnalogClockWidget(QWidget *parent)
    : QWidget(parent)
    , m_timeZone(QTimeZone::systemTimeZone())
    , m_currentUtcTime(QDateTime::currentDateTimeUtc()) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setContextMenuPolicy(Qt::NoContextMenu);
    setCursor(Qt::OpenHandCursor);
}

void AnalogClockWidget::setTimeZone(const QTimeZone &timeZone) {
    if (m_timeZone != timeZone) {
        m_timeZone = timeZone;
        update();
    }
}

QTimeZone AnalogClockWidget::timeZone() const {
    return m_timeZone;
}

void AnalogClockWidget::setTime(const QDateTime &utcNow) {
    m_currentUtcTime = utcNow;
    update();
}

QDateTime AnalogClockWidget::localTime() const {
    return m_currentUtcTime.toTimeZone(m_timeZone);
}

void AnalogClockWidget::setShowSeconds(bool show) {
    if (m_showSeconds != show) {
        m_showSeconds = show;
        update();
    }
}

bool AnalogClockWidget::showSeconds() const {
    return m_showSeconds;
}

void AnalogClockWidget::setShowDayNightShading(bool show) {
    if (m_showDayNightShading != show) {
        m_showDayNightShading = show;
        update();
    }
}

bool AnalogClockWidget::showDayNightShading() const {
    return m_showDayNightShading;
}

void AnalogClockWidget::setWorkingHours(const WorkingHours &hours) {
    if (m_workingHours != hours) {
        m_workingHours = hours;
        update();
    }
}

WorkingHours AnalogClockWidget::workingHours() const {
    return m_workingHours;
}

void AnalogClockWidget::setInteractive(bool interactive) {
    if (m_interactive != interactive) {
        m_interactive = interactive;
        m_isDragging = false;
        m_pendingSeconds = 0.0;
        setAttribute(Qt::WA_TransparentForMouseEvents, !interactive);
        setCursor(interactive ? Qt::OpenHandCursor : Qt::ArrowCursor);
    }
}

bool AnalogClockWidget::isInteractive() const {
    return m_interactive;
}

bool AnalogClockWidget::isNightTime() const {
    return !m_workingHours.isWorkingHour(localTime().time());
}

QSize AnalogClockWidget::sizeHint() const {
    return {180, 180};
}

QSize AnalogClockWidget::minimumSizeHint() const {
    return {80, 80};
}

void AnalogClockWidget::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int side = qMin(width(), height());
    const double radius = (side / 2.0) - 6.0;
    if (radius <= 0) {
        return;
    }

    const bool night = m_showDayNightShading && isNightTime();
    const QTime time = localTime().time();

    painter.save();
    painter.translate(width() / 2.0, height() / 2.0);

    drawDial(painter, radius, night);
    drawTicks(painter, radius, night);
    drawHands(painter, radius, time, night);
    drawAmPmIndicator(painter, radius, time);

    painter.restore();
}

void AnalogClockWidget::drawDial(QPainter &painter, double radius, bool isNight) {
    // Background dial fill
    QRadialGradient dialGradient(0, 0, radius);
    if (isNight) {
        dialGradient.setColorAt(0.0, QColor(0x1e, 0x29, 0x3b)); // Slate 800
        dialGradient.setColorAt(0.85, QColor(0x13, 0x1a, 0x27));
        dialGradient.setColorAt(1.0, QColor(0x0f, 0x17, 0x2a));  // Slate 900
    } else {
        dialGradient.setColorAt(0.0, QColor(0xff, 0xff, 0xff));
        dialGradient.setColorAt(0.85, QColor(0xf8, 0xfa, 0xfc)); // Slate 50
        dialGradient.setColorAt(1.0, QColor(0xee, 0xf2, 0xf6));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(dialGradient);
    painter.drawEllipse(QPointF(0, 0), radius, radius);

    // Subtle bezel border
    const QColor bezelColor = isNight ? QColor(0x47, 0x55, 0x69, 180) : QColor(0xcb, 0xd5, 0xe1, 200);
    painter.setPen(QPen(bezelColor, qMax(1.5, radius * 0.02)));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPointF(0, 0), radius, radius);
}

void AnalogClockWidget::drawTicks(QPainter &painter, double radius, bool isNight) {
    const QColor majorColor = isNight ? QColor(0xf1, 0xf5, 0xf9) : QColor(0x1e, 0x29, 0x3b);
    const QColor minorColor = isNight ? QColor(0x64, 0x74, 0x8b, 150) : QColor(0x94, 0xa3, 0xb8, 180);

    const double majorLength = radius * 0.10;
    const double minorLength = radius * 0.05;
    const double majorWidth = qMax(2.0, radius * 0.025);
    const double minorWidth = qMax(1.0, radius * 0.012);

    for (int i = 0; i < 60; ++i) {
        painter.save();
        painter.rotate(i * 6.0);

        if (i % 5 == 0) {
            painter.setPen(QPen(majorColor, majorWidth, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(0, -radius + 3.0), QPointF(0, -radius + 3.0 + majorLength));
        } else {
            painter.setPen(QPen(minorColor, minorWidth, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(0, -radius + 3.0), QPointF(0, -radius + 3.0 + minorLength));
        }

        painter.restore();
    }
}

void AnalogClockWidget::drawHands(QPainter &painter, double radius, const QTime &time, bool isNight) {
    const double hour = time.hour() % 12 + time.minute() / 60.0 + time.second() / 3600.0;
    const double minute = time.minute() + time.second() / 60.0;
    const double second = time.second();

    const QColor hourColor = isNight ? QColor(0xf8, 0xfa, 0xfc) : QColor(0x0f, 0x17, 0x2a);
    const QColor minColor = isNight ? QColor(0xcb, 0xd5, 0xe1) : QColor(0x33, 0x41, 0x55);
    const QColor secColor = isNight ? QColor(0xfb, 0x92, 0x3c) : QColor(0xea, 0x58, 0x0c); // Orange accent

    // 1. Hour hand
    painter.save();
    painter.rotate(hour * 30.0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(hourColor);

    const double hourLen = radius * 0.55;
    const double hourHalfW = qMax(2.5, radius * 0.038);
    QPolygonF hourPolygon;
    hourPolygon << QPointF(-hourHalfW, radius * 0.08)
                << QPointF(-hourHalfW * 0.6, -hourLen)
                << QPointF(0, -hourLen - hourHalfW * 0.4)
                << QPointF(hourHalfW * 0.6, -hourLen)
                << QPointF(hourHalfW, radius * 0.08);
    painter.drawPolygon(hourPolygon);
    painter.restore();

    // 2. Minute hand
    painter.save();
    painter.rotate(minute * 6.0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(minColor);

    const double minLen = radius * 0.78;
    const double minHalfW = qMax(1.8, radius * 0.026);
    QPolygonF minPolygon;
    minPolygon << QPointF(-minHalfW, radius * 0.08)
               << QPointF(-minHalfW * 0.5, -minLen)
               << QPointF(0, -minLen - minHalfW * 0.5)
               << QPointF(minHalfW * 0.5, -minLen)
               << QPointF(minHalfW, radius * 0.08);
    painter.drawPolygon(minPolygon);
    painter.restore();

    // 3. Second hand (slender with counterweight)
    if (m_showSeconds) {
        painter.save();
        painter.rotate(second * 6.0);
        painter.setPen(QPen(secColor, qMax(1.2, radius * 0.015), Qt::SolidLine, Qt::RoundCap));

        const double secLen = radius * 0.86;
        const double counterLen = radius * 0.18;
        painter.drawLine(QPointF(0, counterLen), QPointF(0, -secLen));

        // Counterweight dot
        painter.setBrush(secColor);
        painter.drawEllipse(QPointF(0, counterLen * 0.6), radius * 0.03, radius * 0.03);
        painter.restore();
    }

    // 4. Center pivot cap
    painter.setPen(QPen(secColor, 1.0));
    painter.setBrush(secColor);
    const double pivotRadius = qMax(3.0, radius * 0.045);
    painter.drawEllipse(QPointF(0, 0), pivotRadius, pivotRadius);

    painter.setPen(Qt::NoPen);
    painter.setBrush(isNight ? QColor(0x0f, 0x17, 0x2a) : QColor(0xff, 0xff, 0xff));
    painter.drawEllipse(QPointF(0, 0), pivotRadius * 0.35, pivotRadius * 0.35);
}

void AnalogClockWidget::drawAmPmIndicator(QPainter &painter, double radius, const QTime &time) {
    const bool isPm = time.hour() >= 12;
    const QString text = isPm ? QStringLiteral("PM") : QStringLiteral("AM");
    const QColor color = isPm ? QColor(0x1e, 0x3a, 0x8a) : QColor(0x02, 0x84, 0xc7);

    QFont font = painter.font();
    font.setBold(true);
    font.setPointSizeF(qBound(8.0, radius * 0.13, 16.0));
    painter.setFont(font);
    painter.setPen(color);

    const double halfSide = radius + 6.0;
    const double inset = 2.0;
    const QRectF cornerRect = isPm
        ? QRectF(0.0, -halfSide + inset, halfSide - inset, halfSide - inset)
        : QRectF(-halfSide + inset, -halfSide + inset, halfSide - inset, halfSide - inset);
    const Qt::Alignment alignment = isPm ? (Qt::AlignRight | Qt::AlignTop) : (Qt::AlignLeft | Qt::AlignTop);

    painter.drawText(cornerRect, alignment, text);
}

double AnalogClockWidget::angleFromCenter(const QPointF &pos) const {
    const double dx = pos.x() - width() / 2.0;
    const double dy = pos.y() - height() / 2.0;
    return std::atan2(dx, -dy);
}

double AnalogClockWidget::normalizeAngleDelta(double delta) {
    constexpr double twoPi = 2.0 * M_PI;
    while (delta > M_PI) {
        delta -= twoPi;
    }
    while (delta <= -M_PI) {
        delta += twoPi;
    }
    return delta;
}

bool AnalogClockWidget::isNearHourHand(const QPointF &pos, double angle) const {
    const double side = qMin(width(), height());
    const double radius = (side / 2.0) - 6.0;
    if (radius <= 0.0) {
        return false;
    }

    const double dx = pos.x() - width() / 2.0;
    const double dy = pos.y() - height() / 2.0;
    const double dist = std::hypot(dx, dy);
    if (dist > radius * 0.65) {
        return false;
    }

    const QTime time = localTime().time();
    const double hourAngle = ((time.hour() % 12) + time.minute() / 60.0 + time.second() / 3600.0) * (2.0 * M_PI / 12.0);
    const double minuteAngle = (time.minute() + time.second() / 60.0) * (2.0 * M_PI / 60.0);

    const double hourDiff = std::abs(normalizeAngleDelta(angle - hourAngle));
    const double minDiff = std::abs(normalizeAngleDelta(angle - minuteAngle));
    return hourDiff < 0.45 && hourDiff <= minDiff;
}

void AnalogClockWidget::mousePressEvent(QMouseEvent *event) {
    if (!m_interactive || event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    m_isDragging = true;
    m_pendingSeconds = 0.0;
    m_lastDragAngle = angleFromCenter(event->position());
    m_secondsPerTurn = isNearHourHand(event->position(), m_lastDragAngle) ? 43200.0 : 3600.0;
    setCursor(Qt::ClosedHandCursor);
    event->accept();
}

void AnalogClockWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!m_isDragging) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const double angle = angleFromCenter(event->position());
    const double delta = normalizeAngleDelta(angle - m_lastDragAngle);
    m_lastDragAngle = angle;

    m_pendingSeconds += (delta / (2.0 * M_PI)) * m_secondsPerTurn;
    const auto wholeSeconds = static_cast<qint64>(std::trunc(m_pendingSeconds));
    if (wholeSeconds != 0) {
        m_pendingSeconds -= static_cast<double>(wholeSeconds);
        TimeEngine::instance().adjustTimeBySeconds(wholeSeconds);
    }
    event->accept();
}

void AnalogClockWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (m_isDragging && event->button() == Qt::LeftButton) {
        m_isDragging = false;
        m_pendingSeconds = 0.0;
        setCursor(m_interactive ? Qt::OpenHandCursor : Qt::ArrowCursor);
        if (TimeEngine::instance().isPreviewActive()) {
            TimeEngine::instance().adjustTimeBySeconds(0);
        }
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AnalogClockWidget::wheelEvent(QWheelEvent *event) {
    if (!m_interactive) {
        QWidget::wheelEvent(event);
        return;
    }

    const int delta = event->angleDelta().y();
    if (delta == 0) {
        QWidget::wheelEvent(event);
        return;
    }

    double secondsPerNotch = 900.0; // 15 minutes per wheel notch
    if (event->modifiers() & Qt::ShiftModifier) {
        secondsPerNotch = 60.0;
    } else if (event->modifiers() & Qt::ControlModifier) {
        secondsPerNotch = 3600.0;
    }

    const auto deltaSeconds = static_cast<qint64>(std::llround((delta / 120.0) * secondsPerNotch));
    if (deltaSeconds != 0) {
        TimeEngine::instance().adjustTimeBySeconds(deltaSeconds);
    }
    event->accept();
}

} // namespace qworldclock

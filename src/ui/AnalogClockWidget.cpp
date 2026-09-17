#include "ui/AnalogClockWidget.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>

namespace qworldclock {

AnalogClockWidget::AnalogClockWidget(QWidget *parent)
    : QWidget(parent)
    , m_timeZone(QTimeZone::systemTimeZone())
    , m_currentUtcTime(QDateTime::currentDateTimeUtc()) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
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

bool AnalogClockWidget::isNightTime() const {
    const int hour = localTime().time().hour();
    return hour >= 20 || hour < 6;
}

QSize AnalogClockWidget::sizeHint() const {
    return {180, 180};
}

QSize AnalogClockWidget::minimumSizeHint() const {
    return {80, 80};
}

bool AnalogClockWidget::hasHeightForWidth() const {
    return true;
}

int AnalogClockWidget::heightForWidth(int w) const {
    return w;
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

} // namespace qworldclock

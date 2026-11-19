#pragma once

#include "core/WorkingHours.hpp"

#include <QDateTime>
#include <QTimeZone>
#include <QWidget>

namespace qworldclock {

class AnalogClockWidget : public QWidget {
    Q_OBJECT

public:
    explicit AnalogClockWidget(QWidget *parent = nullptr);
    ~AnalogClockWidget() override = default;

    void setTimeZone(const QTimeZone &timeZone);
    [[nodiscard]] QTimeZone timeZone() const;

    void setTime(const QDateTime &utcNow);
    [[nodiscard]] QDateTime localTime() const;

    void setShowSeconds(bool show);
    [[nodiscard]] bool showSeconds() const;

    void setShowDayNightShading(bool show);
    [[nodiscard]] bool showDayNightShading() const;

    void setWorkingHours(const WorkingHours &hours);
    [[nodiscard]] WorkingHours workingHours() const;

    void setInteractive(bool interactive);
    [[nodiscard]] bool isInteractive() const;

    [[nodiscard]] bool isNightTime() const;

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void drawDial(QPainter &painter, double radius, bool isNight);
    void drawTicks(QPainter &painter, double radius, bool isNight);
    void drawHands(QPainter &painter, double radius, const QTime &time, bool isNight);
    void drawAmPmIndicator(QPainter &painter, double radius, const QTime &time);
    [[nodiscard]] double angleFromCenter(const QPointF &pos) const;
    [[nodiscard]] static double normalizeAngleDelta(double delta);
    [[nodiscard]] bool isNearHourHand(const QPointF &pos, double angle) const;

    QTimeZone m_timeZone;
    QDateTime m_currentUtcTime;
    bool m_showSeconds{true};
    bool m_showDayNightShading{true};
    bool m_interactive{true};
    bool m_isDragging{false};
    double m_lastDragAngle{0.0};
    double m_secondsPerTurn{3600.0};
    double m_pendingSeconds{0.0};
    WorkingHours m_workingHours{8, 0, 18, 0};
};

} // namespace qworldclock

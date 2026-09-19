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

    [[nodiscard]] bool isNightTime() const;

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawDial(QPainter &painter, double radius, bool isNight);
    void drawTicks(QPainter &painter, double radius, bool isNight);
    void drawHands(QPainter &painter, double radius, const QTime &time, bool isNight);

    QTimeZone m_timeZone;
    QDateTime m_currentUtcTime;
    bool m_showSeconds{true};
    bool m_showDayNightShading{true};
    WorkingHours m_workingHours{8, 0, 18, 0};
};

} // namespace qworldclock

#pragma once

#include <QTime>
#include <QString>

namespace qworldclock {

struct WorkingHours {
    QTime startTime{8, 0};
    QTime endTime{18, 0};

    WorkingHours() = default;
    WorkingHours(const QTime &start, const QTime &end)
        : startTime(start), endTime(end) {}
    WorkingHours(int startHour, int startMin, int endHour, int endMin)
        : startTime(startHour, startMin), endTime(endHour, endMin) {}

    [[nodiscard]] bool isWorkingHour(const QTime &time) const {
        if (!startTime.isValid() || !endTime.isValid()) {
            return false;
        }
        if (startTime == endTime) {
            return false;
        }
        if (startTime < endTime) {
            return time >= startTime && time < endTime;
        } else {
            return time >= startTime || time < endTime;
        }
    }

    [[nodiscard]] QString formatRange() const {
        return QStringLiteral("%1 - %2")
            .arg(startTime.toString(QStringLiteral("HH:mm")),
                 endTime.toString(QStringLiteral("HH:mm")));
    }

    bool operator==(const WorkingHours &other) const {
        return startTime == other.startTime && endTime == other.endTime;
    }

    bool operator!=(const WorkingHours &other) const {
        return !(*this == other);
    }
};

} // namespace qworldclock

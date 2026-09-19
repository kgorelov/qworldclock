#pragma once

#include "core/WorkingHours.hpp"

#include <QObject>
#include <QString>
#include <QTimeZone>
#include <optional>
#include <vector>

namespace qworldclock {

enum class Direction {
    Left,
    Right,
    Above,
    Below
};

struct ClockItem {
    QString id;
    QTimeZone timeZone;
    QString caption;
    int row{0};
    int col{0};
    bool hasCustomWorkingHours{false};
    WorkingHours customWorkingHours{8, 0, 18, 0};
    bool hasCustomCaptionFontSize{false};
    int customCaptionFontSize{0};

    bool operator==(const ClockItem &other) const {
        return id == other.id;
    }
};

class GridModel : public QObject {
    Q_OBJECT

public:
    explicit GridModel(QObject *parent = nullptr);
    ~GridModel() override = default;

    // Clock operations
    bool addClock(const ClockItem &item);
    bool insertRelative(const QString &referenceId, Direction direction, const ClockItem &newItem);
    bool removeClock(const QString &id);
    bool swapClocks(const QString &id1, const QString &id2);
    bool moveClock(const QString &id, int targetRow, int targetCol);
    bool setClockWorkingHours(const QString &id, bool hasCustom, const WorkingHours &hours = WorkingHours());
    bool setClockCaptionFontSize(const QString &id, bool hasCustom, int size = 0);
    void clear();

    // Queries
    [[nodiscard]] int count() const;
    [[nodiscard]] bool canRemove() const;
    [[nodiscard]] const std::vector<ClockItem> &clocks() const;
    [[nodiscard]] std::optional<ClockItem> clockById(const QString &id) const;
    [[nodiscard]] std::optional<ClockItem> clockAt(int row, int col) const;
    [[nodiscard]] int rowCount() const;
    [[nodiscard]] int columnCount() const;

    // Normalizes row/column indices to eliminate empty rows/columns starting at 0
    void normalizeCoordinates();

signals:
    void clockAdded(const ClockItem &item);
    void clockRemoved(const QString &id);
    void layoutChanged();

private:
    std::vector<ClockItem>::iterator findById(const QString &id);
    std::vector<ClockItem>::const_iterator findById(const QString &id) const;

    std::vector<ClockItem> m_clocks;
};

} // namespace qworldclock

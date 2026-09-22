#include "core/GridModel.hpp"

#include <algorithm>
#include <map>
#include <set>

namespace qworldclock {

GridModel::GridModel(QObject *parent)
    : QObject(parent) {
}

bool GridModel::addClock(const ClockItem &item) {
    if (findById(item.id) != m_clocks.end()) {
        return false;
    }

    ClockItem toAdd = item;
    toAdd.row = std::max(0, toAdd.row);
    toAdd.col = std::max(0, toAdd.col);

    // If target position is already occupied, find next available col in that row
    while (clockAt(toAdd.row, toAdd.col).has_value()) {
        toAdd.col++;
    }

    m_clocks.push_back(toAdd);
    emit clockAdded(m_clocks.back());
    emit layoutChanged();
    return true;
}

bool GridModel::setClocks(const std::vector<ClockItem> &items) {
    m_clocks.clear();
    for (const auto &item : items) {
        if (findById(item.id) == m_clocks.end() && !item.id.isEmpty()) {
            ClockItem toAdd = item;
            toAdd.row = std::max(0, toAdd.row);
            toAdd.col = std::max(0, toAdd.col);
            m_clocks.push_back(toAdd);
        }
    }
    normalizeCoordinates();
    emit layoutChanged();
    return true;
}

bool GridModel::setClocks(const QList<ClockItem> &items) {
    std::vector<ClockItem> vec;
    vec.reserve(items.size());
    for (const auto &item : items) {
        vec.push_back(item);
    }
    return setClocks(vec);
}

bool GridModel::insertRelative(const QString &referenceId, Direction direction, const ClockItem &newItem) {
    if (findById(newItem.id) != m_clocks.end()) {
        return false;
    }

    auto refIt = findById(referenceId);
    if (refIt == m_clocks.end()) {
        return false;
    }

    const int refRow = refIt->row;
    const int refCol = refIt->col;
    ClockItem inserted = newItem;

    switch (direction) {
    case Direction::Right: {
        const int targetRow = refRow;
        const int targetCol = refCol + 1;
        // If target cell is already occupied, shift all columns to the right of refCol
        bool occupied = std::any_of(m_clocks.begin(), m_clocks.end(), [targetRow, targetCol](const ClockItem &c) {
            return c.row == targetRow && c.col == targetCol;
        });
        if (occupied) {
            for (auto &clock : m_clocks) {
                if (clock.col > refCol) {
                    clock.col++;
                }
            }
        }
        inserted.row = targetRow;
        inserted.col = targetCol;
        break;
    }
    case Direction::Left: {
        // Shift all columns at and to the right of refCol
        for (auto &clock : m_clocks) {
            if (clock.col >= refCol) {
                clock.col++;
            }
        }
        inserted.row = refRow;
        inserted.col = refCol;
        break;
    }
    case Direction::Below: {
        const int targetRow = refRow + 1;
        const int targetCol = refCol;
        // If target cell is already occupied, shift all rows below refRow
        bool occupied = std::any_of(m_clocks.begin(), m_clocks.end(), [targetRow, targetCol](const ClockItem &c) {
            return c.row == targetRow && c.col == targetCol;
        });
        if (occupied) {
            for (auto &clock : m_clocks) {
                if (clock.row > refRow) {
                    clock.row++;
                }
            }
        }
        inserted.row = targetRow;
        inserted.col = targetCol;
        break;
    }
    case Direction::Above: {
        // Shift all rows at and below refRow
        for (auto &clock : m_clocks) {
            if (clock.row >= refRow) {
                clock.row++;
            }
        }
        inserted.row = refRow;
        inserted.col = refCol;
        break;
    }
    }

    m_clocks.push_back(inserted);
    normalizeCoordinates();
    emit clockAdded(m_clocks.back());
    emit layoutChanged();
    return true;
}

bool GridModel::removeClock(const QString &id) {
    if (!canRemove()) {
        return false;
    }

    auto it = findById(id);
    if (it == m_clocks.end()) {
        return false;
    }

    m_clocks.erase(it);
    normalizeCoordinates();
    emit clockRemoved(id);
    emit layoutChanged();
    return true;
}

bool GridModel::swapClocks(const QString &id1, const QString &id2) {
    if (id1 == id2) {
        return false;
    }

    auto it1 = findById(id1);
    auto it2 = findById(id2);
    if (it1 == m_clocks.end() || it2 == m_clocks.end()) {
        return false;
    }

    std::swap(it1->row, it2->row);
    std::swap(it1->col, it2->col);
    normalizeCoordinates();
    emit layoutChanged();
    return true;
}

bool GridModel::moveClock(const QString &id, int targetRow, int targetCol) {
    auto it = findById(id);
    if (it == m_clocks.end()) {
        return false;
    }

    // If another clock already occupies target position, swap them
    auto targetIt = std::find_if(m_clocks.begin(), m_clocks.end(), [targetRow, targetCol](const ClockItem &c) {
        return c.row == targetRow && c.col == targetCol;
    });

    if (targetIt != m_clocks.end() && targetIt != it) {
        std::swap(it->row, targetIt->row);
        std::swap(it->col, targetIt->col);
    } else {
        it->row = targetRow;
        it->col = targetCol;
    }

    normalizeCoordinates();
    emit layoutChanged();
    return true;
}

bool GridModel::setClockWorkingHours(const QString &id, bool hasCustom, const WorkingHours &hours) {
    auto it = findById(id);
    if (it == m_clocks.end()) {
        return false;
    }

    it->hasCustomWorkingHours = hasCustom;
    if (hasCustom) {
        it->customWorkingHours = hours;
    }
    emit layoutChanged();
    return true;
}

bool GridModel::setClockCaptionFontSize(const QString &id, bool hasCustom, int size) {
    auto it = findById(id);
    if (it == m_clocks.end()) {
        return false;
    }

    it->hasCustomCaptionFontSize = hasCustom;
    if (hasCustom) {
        it->customCaptionFontSize = size;
    }
    emit layoutChanged();
    return true;
}

void GridModel::clear() {
    if (!m_clocks.empty()) {
        m_clocks.clear();
        emit layoutChanged();
    }
}

int GridModel::count() const {
    return static_cast<int>(m_clocks.size());
}

bool GridModel::canRemove() const {
    return m_clocks.size() > 1;
}

const std::vector<ClockItem> &GridModel::clocks() const {
    return m_clocks;
}

std::optional<ClockItem> GridModel::clockById(const QString &id) const {
    auto it = findById(id);
    if (it != m_clocks.end()) {
        return *it;
    }
    return std::nullopt;
}

std::optional<ClockItem> GridModel::clockAt(int row, int col) const {
    auto it = std::find_if(m_clocks.begin(), m_clocks.end(), [row, col](const ClockItem &c) {
        return c.row == row && c.col == col;
    });
    if (it != m_clocks.end()) {
        return *it;
    }
    return std::nullopt;
}

int GridModel::rowCount() const {
    if (m_clocks.empty()) {
        return 0;
    }
    int maxRow = 0;
    for (const auto &clock : m_clocks) {
        maxRow = std::max(maxRow, clock.row);
    }
    return maxRow + 1;
}

int GridModel::columnCount() const {
    if (m_clocks.empty()) {
        return 0;
    }
    int maxCol = 0;
    for (const auto &clock : m_clocks) {
        maxCol = std::max(maxCol, clock.col);
    }
    return maxCol + 1;
}

void GridModel::normalizeCoordinates() {
    if (m_clocks.empty()) {
        return;
    }

    // 1. Resolve duplicate/overlapping coordinates (e.g. recovering from corrupted configs)
    std::set<std::pair<int, int>> occupied;
    for (auto &clock : m_clocks) {
        clock.row = std::max(0, clock.row);
        clock.col = std::max(0, clock.col);
        if (occupied.contains({clock.row, clock.col})) {
            int newCol = clock.col + 1;
            while (occupied.contains({clock.row, newCol}) ||
                   std::any_of(m_clocks.begin(), m_clocks.end(), [&](const ClockItem &c) {
                       return &c != &clock && c.row == clock.row && c.col == newCol;
                   })) {
                newCol++;
            }
            clock.col = newCol;
        }
        occupied.insert({clock.row, clock.col});
    }

    // 2. Eliminate gaps in row and column indices
    std::set<int> uniqueRows;
    std::set<int> uniqueCols;
    for (const auto &clock : m_clocks) {
        uniqueRows.insert(clock.row);
        uniqueCols.insert(clock.col);
    }

    std::map<int, int> rowMapping;
    int normalizedRow = 0;
    for (int r : uniqueRows) {
        rowMapping[r] = normalizedRow++;
    }

    std::map<int, int> colMapping;
    int normalizedCol = 0;
    for (int c : uniqueCols) {
        colMapping[c] = normalizedCol++;
    }

    for (auto &clock : m_clocks) {
        clock.row = rowMapping[clock.row];
        clock.col = colMapping[clock.col];
    }

    // 3. Keep m_clocks in consistent row-major order
    std::sort(m_clocks.begin(), m_clocks.end(), [](const ClockItem &a, const ClockItem &b) {
        if (a.row != b.row) {
            return a.row < b.row;
        }
        return a.col < b.col;
    });
}

std::vector<ClockItem>::iterator GridModel::findById(const QString &id) {
    return std::find_if(m_clocks.begin(), m_clocks.end(), [&id](const ClockItem &c) {
        return c.id == id;
    });
}

std::vector<ClockItem>::const_iterator GridModel::findById(const QString &id) const {
    return std::find_if(m_clocks.begin(), m_clocks.end(), [&id](const ClockItem &c) {
        return c.id == id;
    });
}

} // namespace qworldclock

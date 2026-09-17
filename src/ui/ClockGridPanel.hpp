#pragma once

#include "core/GridModel.hpp"

#include <QMap>
#include <QWidget>

class QGridLayout;
class QHBoxLayout;
class QSpacerItem;

namespace qworldclock {

class ClockCardWidget;

class ClockGridPanel : public QWidget {
    Q_OBJECT

public:
    explicit ClockGridPanel(GridModel *model, QWidget *parent = nullptr);
    ~ClockGridPanel() override = default;

    [[nodiscard]] GridModel *model() const;

    void setGridAlignment(Qt::Alignment alignment);
    [[nodiscard]] Qt::Alignment gridAlignment() const;

    void setEditMode(bool editMode);
    [[nodiscard]] bool isEditMode() const;

    // Helper methods for adding / removing clocks
    bool addClockRelative(const QString &refId, Direction direction, const QTimeZone &timeZone, const QString &caption);
    bool removeClock(const QString &id);

    // Direct access to card widget by ID
    [[nodiscard]] ClockCardWidget *cardWidget(const QString &id) const;

signals:
    void clockCountChanged(int newCount);
    void requestAddClock(const QString &refId, Direction direction);

public slots:
    void refreshLayout();

private:
    void setupUi();
    ClockCardWidget *createCardWidget(const ClockItem &item);
    void updateAlignmentSpacers();

    GridModel *m_model{nullptr};
    QMap<QString, ClockCardWidget *> m_cardWidgets;

    QHBoxLayout *m_outerLayout{nullptr};
    QWidget *m_gridContainer{nullptr};
    QGridLayout *m_gridLayout{nullptr};

    Qt::Alignment m_alignment{Qt::AlignCenter};
    bool m_editMode{false};
};

} // namespace qworldclock

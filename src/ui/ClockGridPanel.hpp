#pragma once

#include "core/GridModel.hpp"

#include <QMap>
#include <QWidget>

class QGridLayout;
class QHBoxLayout;

namespace qworldclock {

class ClockCardWidget;

enum class SizingMode {
    Responsive,
    Fixed
};

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

    void setSizingMode(SizingMode mode);
    [[nodiscard]] SizingMode sizingMode() const;

    void setFixedClockSize(int size);
    [[nodiscard]] int fixedClockSize() const;

    void setGlobalWorkingHours(const WorkingHours &hours);
    [[nodiscard]] WorkingHours globalWorkingHours() const;

    // Helper methods for adding / removing clocks
    bool addClockRelative(const QString &refId, Direction direction, const QTimeZone &timeZone, const QString &caption);
    bool removeClock(const QString &id);

    // Direct access to card widget by ID
    [[nodiscard]] ClockCardWidget *cardWidget(const QString &id) const;

    void updateCardSizes();

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

signals:
    void clockCountChanged(int newCount);
    void requestAddClock(const QString &refId, Direction direction);
    void requestConfigureClockWorkingHours(const QString &clockId);
    void requestResetClockWorkingHours(const QString &clockId);
    void requestGlobalWorkingHours();
    void globalWorkingHoursChanged(const WorkingHours &hours);

public slots:
    void refreshLayout();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUi();
    ClockCardWidget *createCardWidget(const ClockItem &item);
    void updateAlignment();
    void applySizingToCard(ClockCardWidget *card);

    GridModel *m_model{nullptr};
    QMap<QString, ClockCardWidget *> m_cardWidgets;

    QHBoxLayout *m_outerLayout{nullptr};
    QWidget *m_gridContainer{nullptr};
    QGridLayout *m_gridLayout{nullptr};

    Qt::Alignment m_alignment{Qt::AlignCenter};
    SizingMode m_sizingMode{SizingMode::Responsive};
    int m_fixedClockSize{190};
    bool m_editMode{false};
    WorkingHours m_globalWorkingHours{8, 0, 18, 0};
};

} // namespace qworldclock

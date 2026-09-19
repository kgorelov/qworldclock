#include "ui/ClockGridPanel.hpp"
#include "ui/ClockCardWidget.hpp"

#include <QAction>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QResizeEvent>
#include <QUuid>

namespace qworldclock {

ClockGridPanel::ClockGridPanel(GridModel *model, QWidget *parent)
    : QWidget(parent)
    , m_model(model) {
    setupUi();

    if (m_model) {
        connect(m_model, &GridModel::layoutChanged, this, &ClockGridPanel::refreshLayout);
    }
}

GridModel *ClockGridPanel::model() const {
    return m_model;
}

void ClockGridPanel::setGridAlignment(Qt::Alignment alignment) {
    if (m_alignment != alignment) {
        m_alignment = alignment;
        updateAlignment();
    }
}

Qt::Alignment ClockGridPanel::gridAlignment() const {
    return m_alignment;
}

void ClockGridPanel::setEditMode(bool editMode) {
    if (m_editMode != editMode) {
        m_editMode = editMode;
        for (auto *card : m_cardWidgets) {
            if (card) {
                card->setEditMode(editMode);
            }
        }
    }
}

bool ClockGridPanel::isEditMode() const {
    return m_editMode;
}

void ClockGridPanel::setSizingMode(SizingMode mode) {
    if (m_sizingMode != mode) {
        m_sizingMode = mode;
        updateCardSizes();
    }
}

SizingMode ClockGridPanel::sizingMode() const {
    return m_sizingMode;
}

void ClockGridPanel::setFixedClockSize(int size) {
    const int clamped = qBound(100, size, 500);
    if (m_fixedClockSize != clamped) {
        m_fixedClockSize = clamped;
        if (m_sizingMode == SizingMode::Fixed) {
            updateCardSizes();
        }
    }
}

int ClockGridPanel::fixedClockSize() const {
    return m_fixedClockSize;
}

void ClockGridPanel::setGlobalWorkingHours(const WorkingHours &hours) {
    if (m_globalWorkingHours != hours) {
        m_globalWorkingHours = hours;
        for (auto *card : m_cardWidgets) {
            if (card) {
                card->setGlobalWorkingHours(hours);
            }
        }
        emit globalWorkingHoursChanged(hours);
    }
}

WorkingHours ClockGridPanel::globalWorkingHours() const {
    return m_globalWorkingHours;
}

void ClockGridPanel::applySizingToCard(ClockCardWidget *card) {
    if (!card) {
        return;
    }
    if (m_sizingMode == SizingMode::Fixed) {
        card->setFixedClockSize(m_fixedClockSize);
    } else {
        card->setClockDiameter(card->clockDiameter());
    }
}

ClockCardWidget *ClockGridPanel::cardWidget(const QString &id) const {
    return m_cardWidgets.value(id, nullptr);
}

void ClockGridPanel::updateCardSizes() {
    if (!m_model || m_model->count() == 0) {
        return;
    }

    const int rows = qMax(1, m_model->rowCount());
    const int cols = qMax(1, m_model->columnCount());

    const int extraW = 28;
    const int extraH = 76;
    const int gridSpacing = 24;
    const int outerHMargin = 40;
    const int outerVMargin = 40;

    int D = m_fixedClockSize;

    if (m_sizingMode == SizingMode::Responsive) {
        const int panelW = width();
        const int panelH = height();

        if (panelW > 100 && panelH > 100) {
            const int totalHSpacing = gridSpacing * (cols - 1);
            const int totalVSpacing = gridSpacing * (rows - 1);

            const int availCellW = (panelW - outerHMargin - totalHSpacing) / cols;
            const int availCellH = (panelH - outerVMargin - totalVSpacing) / rows;

            const int maxDFromW = availCellW - extraW;
            const int maxDFromH = availCellH - extraH;
            const int maxD = qMin(maxDFromW, maxDFromH);

            D = qBound(90, maxD, 550);
        } else {
            D = 190;
        }
    }

    const int cardW = D + extraW;
    const int cardH = D + extraH;

    for (auto *card : m_cardWidgets) {
        if (card) {
            card->setClockDiameter(D);
        }
    }

    const int gridW = cols * cardW + (cols - 1) * gridSpacing;
    const int gridH = rows * cardH + (rows - 1) * gridSpacing;

    if (m_gridContainer) {
        m_gridContainer->setFixedSize(gridW, gridH);
    }
    updateAlignment();
}

void ClockGridPanel::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (m_sizingMode == SizingMode::Responsive) {
        updateCardSizes();
    }
}

QSize ClockGridPanel::sizeHint() const {
    if (!m_model || m_model->count() == 0) {
        return {400, 300};
    }
    const int rows = qMax(1, m_model->rowCount());
    const int cols = qMax(1, m_model->columnCount());
    const int baseD = (m_sizingMode == SizingMode::Fixed) ? m_fixedClockSize : 190;
    const int gridW = cols * (baseD + 28) + (cols - 1) * 24 + 40;
    const int gridH = rows * (baseD + 76) + (rows - 1) * 24 + 40;
    return {gridW, gridH};
}

QSize ClockGridPanel::minimumSizeHint() const {
    if (m_sizingMode == SizingMode::Fixed) {
        return sizeHint();
    }
    return {220, 250};
}

void ClockGridPanel::setupUi() {
    m_outerLayout = new QHBoxLayout(this);
    m_outerLayout->setContentsMargins(20, 20, 20, 20);
    m_outerLayout->setSpacing(0);

    m_gridContainer = new QWidget(this);
    m_gridLayout = new QGridLayout(m_gridContainer);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(24);

    m_outerLayout->addWidget(m_gridContainer);

    updateAlignment();
    refreshLayout();
}

void ClockGridPanel::updateAlignment() {
    if (m_outerLayout && m_gridContainer) {
        m_outerLayout->setAlignment(m_gridContainer, m_alignment | Qt::AlignVCenter);
    }
    if (m_gridLayout) {
        m_gridLayout->setAlignment(m_alignment);
    }
}

ClockCardWidget *ClockGridPanel::createCardWidget(const ClockItem &item) {
    auto *card = new ClockCardWidget(item.id, item.timeZone, item.caption, m_gridContainer);
    card->setGridRow(item.row);
    card->setGridCol(item.col);
    card->setEditMode(m_editMode);
    card->setCanRemove(m_model ? m_model->canRemove() : false);
    card->setGlobalWorkingHours(m_globalWorkingHours);
    card->setHasCustomWorkingHours(item.hasCustomWorkingHours);
    if (item.hasCustomWorkingHours) {
        card->setCustomWorkingHours(item.customWorkingHours);
    }
    applySizingToCard(card);

    // Connect edit overlay buttons
    connect(card, &ClockCardWidget::requestAdd, this, &ClockGridPanel::requestAddClock);
    connect(card, &ClockCardWidget::requestRemove, this, [this](const QString &id) {
        removeClock(id);
    });
    connect(card, &ClockCardWidget::clockDropped, this, [this](const QString &srcId, const QString &tgtId) {
        if (m_model) {
            m_model->swapClocks(srcId, tgtId);
        }
    });

    // Provide right-click context menu on each card as well
    card->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(card, &QWidget::customContextMenuRequested, this, [this, card](const QPoint &pos) {
        QMenu menu(card);

        // 1. Working Hours submenu
        auto *whMenu = menu.addMenu(tr("&Working Hours"));

        auto *configWH = whMenu->addAction(tr("&Configure Working Hours..."));
        connect(configWH, &QAction::triggered, this, [this, card]() {
            emit requestConfigureClockWorkingHours(card->clockId());
        });

        auto *useGlobalAction = whMenu->addAction(tr("Use &Global Working Hours (%1)").arg(m_globalWorkingHours.formatRange()));
        useGlobalAction->setCheckable(true);
        useGlobalAction->setChecked(!card->hasCustomWorkingHours());
        connect(useGlobalAction, &QAction::triggered, this, [this, card](bool checked) {
            if (checked) {
                emit requestResetClockWorkingHours(card->clockId());
            } else {
                emit requestConfigureClockWorkingHours(card->clockId());
            }
        });

        whMenu->addSeparator();

        // Start Time submenu
        auto *startMenu = whMenu->addMenu(tr("Override &Start Time"));
        const QTime curStart = card->effectiveWorkingHours().startTime;
        for (int h = 0; h < 24; ++h) {
            const QTime t(h, 0);
            auto *act = startMenu->addAction(t.toString(QStringLiteral("HH:mm")));
            act->setCheckable(true);
            if (card->hasCustomWorkingHours() && curStart.hour() == h && curStart.minute() == 0) {
                act->setChecked(true);
            }
            connect(act, &QAction::triggered, this, [this, card, t]() {
                const WorkingHours cur = card->effectiveWorkingHours();
                const WorkingHours newHours(t, cur.endTime);
                if (m_model) {
                    m_model->setClockWorkingHours(card->clockId(), true, newHours);
                }
                card->setHasCustomWorkingHours(true);
                card->setCustomWorkingHours(newHours);
            });
        }

        // End Time submenu
        auto *endMenu = whMenu->addMenu(tr("Override &End Time"));
        const QTime curEnd = card->effectiveWorkingHours().endTime;
        for (int h = 0; h < 24; ++h) {
            const QTime t(h, 0);
            auto *act = endMenu->addAction(t.toString(QStringLiteral("HH:mm")));
            act->setCheckable(true);
            if (card->hasCustomWorkingHours() && curEnd.hour() == h && curEnd.minute() == 0) {
                act->setChecked(true);
            }
            connect(act, &QAction::triggered, this, [this, card, t]() {
                const WorkingHours cur = card->effectiveWorkingHours();
                const WorkingHours newHours(cur.startTime, t);
                if (m_model) {
                    m_model->setClockWorkingHours(card->clockId(), true, newHours);
                }
                card->setHasCustomWorkingHours(true);
                card->setCustomWorkingHours(newHours);
            });
        }

        whMenu->addSeparator();
        auto addPreset = [this, card, whMenu](const QString &title, int sh, int sm, int eh, int em) {
            auto *act = whMenu->addAction(title);
            connect(act, &QAction::triggered, this, [this, card, sh, sm, eh, em]() {
                const WorkingHours preset(QTime(sh, sm), QTime(eh, em));
                if (m_model) {
                    m_model->setClockWorkingHours(card->clockId(), true, preset);
                }
                card->setHasCustomWorkingHours(true);
                card->setCustomWorkingHours(preset);
            });
        };
        addPreset(tr("08:00 - 18:00 (Standard)"), 8, 0, 18, 0);
        addPreset(tr("09:00 - 17:00 (9 to 5)"), 9, 0, 17, 0);
        addPreset(tr("08:00 - 17:00 (8 to 5)"), 8, 0, 17, 0);
        addPreset(tr("07:00 - 19:00 (Extended)"), 7, 0, 19, 0);

        menu.addSeparator();

        // 2. Direct top-level quick actions
        auto *quickConfig = menu.addAction(tr("&Configure Working Hours..."));
        connect(quickConfig, &QAction::triggered, this, [this, card]() {
            emit requestConfigureClockWorkingHours(card->clockId());
        });

        if (card->hasCustomWorkingHours()) {
            auto *resetAction = menu.addAction(tr("&Reset to Global Working Hours"));
            connect(resetAction, &QAction::triggered, this, [this, card]() {
                emit requestResetClockWorkingHours(card->clockId());
            });
        }

        menu.addSeparator();

        // 3. Grid positioning actions
        auto *addRight = menu.addAction(tr("Add Clock to Right"));
        auto *addLeft = menu.addAction(tr("Add Clock to Left"));
        auto *addBelow = menu.addAction(tr("Add Clock Below"));
        auto *addAbove = menu.addAction(tr("Add Clock Above"));

        connect(addRight, &QAction::triggered, this, [this, card]() {
            emit requestAddClock(card->clockId(), Direction::Right);
        });
        connect(addLeft, &QAction::triggered, this, [this, card]() {
            emit requestAddClock(card->clockId(), Direction::Left);
        });
        connect(addBelow, &QAction::triggered, this, [this, card]() {
            emit requestAddClock(card->clockId(), Direction::Below);
        });
        connect(addAbove, &QAction::triggered, this, [this, card]() {
            emit requestAddClock(card->clockId(), Direction::Above);
        });

        menu.addSeparator();
        auto *removeAction = menu.addAction(tr("Remove Clock"));
        removeAction->setEnabled(m_model && m_model->canRemove());
        connect(removeAction, &QAction::triggered, this, [this, card]() {
            removeClock(card->clockId());
        });

        menu.addSeparator();
        auto *globalWhAction = menu.addAction(tr("&Global Working Hours..."));
        connect(globalWhAction, &QAction::triggered, this, &ClockGridPanel::requestGlobalWorkingHours);

        menu.exec(card->mapToGlobal(pos));
    });

    return card;
}

void ClockGridPanel::refreshLayout() {
    if (!m_model) {
        return;
    }

    const auto &clocks = m_model->clocks();
    QSet<QString> activeIds;
    const bool canRemoveClocks = m_model->canRemove();

    // 1. Position all existing and new cards
    for (const auto &item : clocks) {
        activeIds.insert(item.id);

        ClockCardWidget *card = m_cardWidgets.value(item.id, nullptr);
        if (!card) {
            card = createCardWidget(item);
            m_cardWidgets.insert(item.id, card);
        } else {
            card->setTimeZone(item.timeZone);
            card->setCaption(item.caption);
            card->setGridRow(item.row);
            card->setGridCol(item.col);
            card->setEditMode(m_editMode);
            card->setCanRemove(canRemoveClocks);
            card->setGlobalWorkingHours(m_globalWorkingHours);
            card->setHasCustomWorkingHours(item.hasCustomWorkingHours);
            if (item.hasCustomWorkingHours) {
                card->setCustomWorkingHours(item.customWorkingHours);
            }
            applySizingToCard(card);
        }

        m_gridLayout->addWidget(card, item.row, item.col);
    }

    // 2. Remove widgets for deleted clocks
    const auto existingIds = m_cardWidgets.keys();
    for (const auto &id : existingIds) {
        if (!activeIds.contains(id)) {
            auto *card = m_cardWidgets.take(id);
            m_gridLayout->removeWidget(card);
            card->deleteLater();
        }
    }

    // Update canRemove state across remaining cards
    for (auto *card : m_cardWidgets) {
        if (card) {
            card->setCanRemove(canRemoveClocks);
        }
    }

    updateCardSizes();
    emit clockCountChanged(m_model->count());
}

bool ClockGridPanel::addClockRelative(const QString &refId, Direction direction,
                                      const QTimeZone &timeZone, const QString &caption) {
    if (!m_model) {
        return false;
    }

    ClockItem newItem;
    newItem.id = QStringLiteral("clock-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces).left(8));
    newItem.timeZone = timeZone;
    newItem.caption = caption;

    return m_model->insertRelative(refId, direction, newItem);
}

bool ClockGridPanel::removeClock(const QString &id) {
    if (!m_model) {
        return false;
    }
    return m_model->removeClock(id);
}

} // namespace qworldclock

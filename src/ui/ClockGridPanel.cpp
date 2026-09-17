#include "ui/ClockGridPanel.hpp"
#include "ui/ClockCardWidget.hpp"

#include <QAction>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMenu>
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
        updateAlignmentSpacers();
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

ClockCardWidget *ClockGridPanel::cardWidget(const QString &id) const {
    return m_cardWidgets.value(id, nullptr);
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

    updateAlignmentSpacers();
    refreshLayout();
}

void ClockGridPanel::updateAlignmentSpacers() {
    if (m_outerLayout) {
        m_outerLayout->setAlignment(m_alignment);
    }
    if (m_gridLayout) {
        m_gridLayout->setAlignment(m_alignment);
    }
}

ClockCardWidget *ClockGridPanel::createCardWidget(const ClockItem &item) {
    auto *card = new ClockCardWidget(item.id, item.timeZone, item.caption, m_gridContainer);
    card->setGridRow(item.row);
    card->setGridCol(item.col);
    card->setMinimumSize(170, 220);
    card->setMaximumSize(320, 380);
    card->setEditMode(m_editMode);
    card->setCanRemove(m_model ? m_model->canRemove() : false);

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

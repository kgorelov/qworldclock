#include "ui/ClockCardWidget.hpp"
#include "core/TimeEngine.hpp"
#include "ui/AnalogClockWidget.hpp"
#include "ui/CaptionLabel.hpp"

#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace qworldclock {

ClockCardWidget::ClockCardWidget(const QString &id,
                                 const QTimeZone &timeZone,
                                 const QString &caption,
                                 QWidget *parent)
    : QFrame(parent)
    , m_clockId(id) {
    setupUi();
    setupEditControls();

    setTimeZone(timeZone);
    setCaption(caption);

    // Synchronize to initial time
    setTime(TimeEngine::instance().currentUtcTime());

    // Connect to central time tick
    connect(&TimeEngine::instance(), &TimeEngine::tick, this, &ClockCardWidget::setTime);

    setAcceptDrops(true);
    updateCardStyle();
}

void ClockCardWidget::setupUi() {
    setObjectName(QStringLiteral("ClockCardWidget"));
    setFrameShape(QFrame::StyledPanel);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 10);
    layout->setSpacing(6);

    m_analogClock = new AnalogClockWidget(this);
    layout->addWidget(m_analogClock, 1);

    m_captionLabel = new CaptionLabel(this);
    layout->addWidget(m_captionLabel, 0);
}

void ClockCardWidget::setupEditControls() {
    const QString btnAddStyle = QStringLiteral(
        "QPushButton {"
        "  background-color: #3b82f6;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 12px;"
        "  font-size: 15px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #2563eb; }"
        "QPushButton:pressed { background-color: #1d4ed8; }");

    const QString btnDeleteStyle = QStringLiteral(
        "QPushButton {"
        "  background-color: #ef4444;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 11px;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #dc2626; }"
        "QPushButton:pressed { background-color: #b91c1c; }"
        "QPushButton:disabled { background-color: #cbd5e1; color: #94a3b8; }");

    // Directional '+' buttons
    m_btnAddTop = new QPushButton(QStringLiteral("+"), this);
    m_btnAddTop->setToolTip(tr("Add Clock Above"));
    m_btnAddTop->setStyleSheet(btnAddStyle);
    m_btnAddTop->setFixedSize(24, 24);
    m_btnAddTop->setCursor(Qt::PointingHandCursor);
    connect(m_btnAddTop, &QPushButton::clicked, this, [this]() {
        emit requestAdd(m_clockId, Direction::Above);
    });

    m_btnAddBottom = new QPushButton(QStringLiteral("+"), this);
    m_btnAddBottom->setToolTip(tr("Add Clock Below"));
    m_btnAddBottom->setStyleSheet(btnAddStyle);
    m_btnAddBottom->setFixedSize(24, 24);
    m_btnAddBottom->setCursor(Qt::PointingHandCursor);
    connect(m_btnAddBottom, &QPushButton::clicked, this, [this]() {
        emit requestAdd(m_clockId, Direction::Below);
    });

    m_btnAddLeft = new QPushButton(QStringLiteral("+"), this);
    m_btnAddLeft->setToolTip(tr("Add Clock to Left"));
    m_btnAddLeft->setStyleSheet(btnAddStyle);
    m_btnAddLeft->setFixedSize(24, 24);
    m_btnAddLeft->setCursor(Qt::PointingHandCursor);
    connect(m_btnAddLeft, &QPushButton::clicked, this, [this]() {
        emit requestAdd(m_clockId, Direction::Left);
    });

    m_btnAddRight = new QPushButton(QStringLiteral("+"), this);
    m_btnAddRight->setToolTip(tr("Add Clock to Right"));
    m_btnAddRight->setStyleSheet(btnAddStyle);
    m_btnAddRight->setFixedSize(24, 24);
    m_btnAddRight->setCursor(Qt::PointingHandCursor);
    connect(m_btnAddRight, &QPushButton::clicked, this, [this]() {
        emit requestAdd(m_clockId, Direction::Right);
    });

    // Delete '✕' button
    m_btnDelete = new QPushButton(QStringLiteral("✕"), this);
    m_btnDelete->setToolTip(tr("Remove Clock"));
    m_btnDelete->setStyleSheet(btnDeleteStyle);
    m_btnDelete->setFixedSize(22, 22);
    m_btnDelete->setCursor(Qt::PointingHandCursor);
    connect(m_btnDelete, &QPushButton::clicked, this, [this]() {
        emit requestRemove(m_clockId);
    });

    // Drag indicator icon
    m_dragIndicator = new QLabel(QStringLiteral("⠿"), this);
    m_dragIndicator->setToolTip(tr("Drag to reorder clock"));
    m_dragIndicator->setStyleSheet(QStringLiteral("QLabel { color: #64748b; font-size: 14px; background: transparent; }"));
    m_dragIndicator->setFixedSize(18, 18);
    m_dragIndicator->setCursor(Qt::SizeAllCursor);

    // Initial visibility: hidden until Edit Mode is activated
    m_btnAddTop->setVisible(false);
    m_btnAddBottom->setVisible(false);
    m_btnAddLeft->setVisible(false);
    m_btnAddRight->setVisible(false);
    m_btnDelete->setVisible(false);
    m_dragIndicator->setVisible(false);
}

void ClockCardWidget::updateButtonPositions() {
    const int w = width();
    const int h = height();

    if (m_btnAddTop) {
        m_btnAddTop->move(w / 2 - 12, 3);
    }
    if (m_btnAddBottom) {
        m_btnAddBottom->move(w / 2 - 12, h - 27);
    }
    if (m_btnAddLeft) {
        m_btnAddLeft->move(3, h / 2 - 12);
    }
    if (m_btnAddRight) {
        m_btnAddRight->move(w - 27, h / 2 - 12);
    }
    if (m_btnDelete) {
        m_btnDelete->move(w - 25, 4);
    }
    if (m_dragIndicator) {
        m_dragIndicator->move(6, 6);
    }
}

void ClockCardWidget::updateCardStyle() {
    if (m_dropHighlighted) {
        setStyleSheet(QStringLiteral(
            "#ClockCardWidget {"
            "  background-color: rgba(220, 252, 231, 0.9);"
            "  border: 2px solid #22c55e;"
            "  border-radius: 12px;"
            "}"));
    } else if (m_editMode) {
        setStyleSheet(QStringLiteral(
            "#ClockCardWidget {"
            "  background-color: rgba(239, 246, 255, 0.85);"
            "  border: 2px dashed #3b82f6;"
            "  border-radius: 12px;"
            "}"));
    } else {
        setStyleSheet(QStringLiteral(
            "#ClockCardWidget {"
            "  background-color: rgba(255, 255, 255, 0.75);"
            "  border: 1px solid #e2e8f0;"
            "  border-radius: 12px;"
            "}"));
    }
}

void ClockCardWidget::setEditMode(bool editMode) {
    if (m_editMode != editMode) {
        m_editMode = editMode;
        m_btnAddTop->setVisible(editMode);
        m_btnAddBottom->setVisible(editMode);
        m_btnAddLeft->setVisible(editMode);
        m_btnAddRight->setVisible(editMode);
        m_btnDelete->setVisible(editMode && m_canRemove);
        m_dragIndicator->setVisible(editMode);
        setCursor(editMode ? Qt::SizeAllCursor : Qt::ArrowCursor);
        updateCardStyle();
    }
}

bool ClockCardWidget::isEditMode() const {
    return m_editMode;
}

void ClockCardWidget::setCanRemove(bool canRemove) {
    m_canRemove = canRemove;
    if (m_btnDelete) {
        m_btnDelete->setEnabled(canRemove);
        m_btnDelete->setVisible(m_editMode && canRemove);
    }
}

bool ClockCardWidget::canRemove() const {
    return m_canRemove;
}

void ClockCardWidget::setDropHighlighted(bool highlighted) {
    if (m_dropHighlighted != highlighted) {
        m_dropHighlighted = highlighted;
        updateCardStyle();
    }
}

QString ClockCardWidget::clockId() const {
    return m_clockId;
}

void ClockCardWidget::setClockId(const QString &id) {
    m_clockId = id;
}

QTimeZone ClockCardWidget::timeZone() const {
    return m_analogClock ? m_analogClock->timeZone() : QTimeZone::systemTimeZone();
}

void ClockCardWidget::setTimeZone(const QTimeZone &timeZone) {
    if (m_analogClock) {
        m_analogClock->setTimeZone(timeZone);
    }
    if (m_captionLabel) {
        m_captionLabel->setTimeZone(timeZone);
    }
}

QString ClockCardWidget::caption() const {
    return m_captionLabel ? m_captionLabel->caption() : QString();
}

void ClockCardWidget::setCaption(const QString &caption) {
    if (m_captionLabel) {
        m_captionLabel->setCaption(caption);
    }
}

int ClockCardWidget::gridRow() const {
    return m_gridRow;
}

void ClockCardWidget::setGridRow(int row) {
    m_gridRow = row;
}

int ClockCardWidget::gridCol() const {
    return m_gridCol;
}

void ClockCardWidget::setGridCol(int col) {
    m_gridCol = col;
}

AnalogClockWidget *ClockCardWidget::analogClock() const {
    return m_analogClock;
}

CaptionLabel *ClockCardWidget::captionLabel() const {
    return m_captionLabel;
}

void ClockCardWidget::setTime(const QDateTime &utcNow) {
    if (m_analogClock) {
        m_analogClock->setTime(utcNow);
    }
    if (m_captionLabel) {
        m_captionLabel->setTime(utcNow);
    }
}

void ClockCardWidget::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    updateButtonPositions();
}

void ClockCardWidget::mousePressEvent(QMouseEvent *event) {
    if (m_editMode && event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
    }
    QFrame::mousePressEvent(event);
}

void ClockCardWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_editMode && (event->buttons() & Qt::LeftButton)) {
        const int distance = (event->pos() - m_dragStartPos).manhattanLength();
        if (distance >= QApplication::startDragDistance()) {
            auto *drag = new QDrag(this);
            auto *mimeData = new QMimeData();
            mimeData->setData(QStringLiteral("application/x-qworldclock-clock-id"), m_clockId.toUtf8());
            drag->setMimeData(mimeData);

            QPixmap pixmap = grab();
            const QSize scaledSize = pixmap.size() * 0.75;
            drag->setPixmap(pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            drag->setHotSpot(event->pos() * 0.75);

            drag->exec(Qt::MoveAction);
            return;
        }
    }
    QFrame::mouseMoveEvent(event);
}

void ClockCardWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (m_editMode && event->mimeData()->hasFormat(QStringLiteral("application/x-qworldclock-clock-id"))) {
        const QString sourceId = QString::fromUtf8(event->mimeData()->data(QStringLiteral("application/x-qworldclock-clock-id")));
        if (sourceId != m_clockId) {
            event->acceptProposedAction();
            setDropHighlighted(true);
            return;
        }
    }
    event->ignore();
}

void ClockCardWidget::dragLeaveEvent(QDragLeaveEvent *event) {
    setDropHighlighted(false);
    event->accept();
}

void ClockCardWidget::dropEvent(QDropEvent *event) {
    setDropHighlighted(false);
    if (m_editMode && event->mimeData()->hasFormat(QStringLiteral("application/x-qworldclock-clock-id"))) {
        const QString sourceId = QString::fromUtf8(event->mimeData()->data(QStringLiteral("application/x-qworldclock-clock-id")));
        if (sourceId != m_clockId) {
            event->acceptProposedAction();
            emit clockDropped(sourceId, m_clockId);
            return;
        }
    }
    event->ignore();
}

} // namespace qworldclock

#include "ui/ClockCardWidget.hpp"
#include "core/TimeEngine.hpp"
#include "ui/AnalogClockWidget.hpp"
#include "ui/CaptionLabel.hpp"

#include <QVBoxLayout>

namespace qworldclock {

ClockCardWidget::ClockCardWidget(const QString &id,
                                 const QTimeZone &timeZone,
                                 const QString &caption,
                                 QWidget *parent)
    : QFrame(parent)
    , m_clockId(id) {
    setupUi();

    setTimeZone(timeZone);
    setCaption(caption);

    // Synchronize to initial time
    setTime(TimeEngine::instance().currentUtcTime());

    // Connect to central time tick
    connect(&TimeEngine::instance(), &TimeEngine::tick, this, &ClockCardWidget::setTime);
}

void ClockCardWidget::setupUi() {
    setObjectName(QStringLiteral("ClockCardWidget"));
    setFrameShape(QFrame::StyledPanel);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 8);
    layout->setSpacing(6);

    m_analogClock = new AnalogClockWidget(this);
    layout->addWidget(m_analogClock, 1);

    m_captionLabel = new CaptionLabel(this);
    layout->addWidget(m_captionLabel, 0);
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

} // namespace qworldclock

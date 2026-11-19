#include "core/TimeEngine.hpp"

#include <QTime>
#include <QTimer>

namespace qworldclock {

TimeEngine &TimeEngine::instance() {
    static TimeEngine s_instance;
    return s_instance;
}

TimeEngine::TimeEngine(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_alignmentTimer(new QTimer(this))
    , m_previewResetTimer(new QTimer(this)) {
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &TimeEngine::onTick);

    m_alignmentTimer->setSingleShot(true);
    connect(m_alignmentTimer, &QTimer::timeout, this, [this]() {
        onTick();
        m_timer->start(1000);
    });

    m_previewResetTimer->setSingleShot(true);
    connect(m_previewResetTimer, &QTimer::timeout, this, &TimeEngine::resetPreviewTime);
}

void TimeEngine::start() {
    if (isRunning()) {
        return;
    }
    alignAndStart();
}

void TimeEngine::stop() {
    m_alignmentTimer->stop();
    m_timer->stop();
    m_previewResetTimer->stop();
}

bool TimeEngine::isRunning() const {
    return m_timer->isActive() || m_alignmentTimer->isActive();
}

QDateTime TimeEngine::currentUtcTime() const {
    return m_previewActive ? m_previewUtcTime : QDateTime::currentDateTimeUtc();
}

void TimeEngine::adjustTimeBySeconds(qint64 deltaSeconds) {
    const QDateTime base = m_previewActive ? m_previewUtcTime : QDateTime::currentDateTimeUtc();
    setPreviewTime(base.addSecs(deltaSeconds));
}

void TimeEngine::setPreviewTime(const QDateTime &utcTime) {
    if (!utcTime.isValid()) {
        return;
    }
    const bool wasActive = m_previewActive;
    m_previewActive = true;
    m_previewUtcTime = utcTime.toUTC();
    m_previewResetTimer->start(m_previewTimeoutMs);
    if (!wasActive) {
        emit previewModeChanged(true);
    }
    emit tick(m_previewUtcTime);
}

void TimeEngine::resetPreviewTime() {
    m_previewResetTimer->stop();
    if (!m_previewActive) {
        return;
    }
    m_previewActive = false;
    m_previewUtcTime = QDateTime();
    emit previewModeChanged(false);
    emit tick(QDateTime::currentDateTimeUtc());
}

bool TimeEngine::isPreviewActive() const {
    return m_previewActive;
}

void TimeEngine::setPreviewTimeoutMs(int ms) {
    m_previewTimeoutMs = qMax(1, ms);
}

void TimeEngine::alignAndStart() {
    // Fire an initial tick immediately
    onTick();

    // Calculate milliseconds until the next second boundary
    const int currentMsec = QTime::currentTime().msec();
    int msToNextSecond = 1000 - currentMsec;
    if (msToNextSecond <= 0 || msToNextSecond > 1000) {
        msToNextSecond = 1000;
    }

    m_alignmentTimer->start(msToNextSecond);
}

void TimeEngine::onTick() {
    if (m_previewActive) {
        return;
    }
    emit tick(QDateTime::currentDateTimeUtc());
}

} // namespace qworldclock

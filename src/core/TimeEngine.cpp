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
    , m_alignmentTimer(new QTimer(this)) {
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &TimeEngine::onTick);

    m_alignmentTimer->setSingleShot(true);
    connect(m_alignmentTimer, &QTimer::timeout, this, [this]() {
        onTick();
        m_timer->start(1000);
    });
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
}

bool TimeEngine::isRunning() const {
    return m_timer->isActive() || m_alignmentTimer->isActive();
}

QDateTime TimeEngine::currentUtcTime() const {
    return QDateTime::currentDateTimeUtc();
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
    emit tick(QDateTime::currentDateTimeUtc());
}

} // namespace qworldclock

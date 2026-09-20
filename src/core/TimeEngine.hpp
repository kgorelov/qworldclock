#pragma once

#include <QDateTime>
#include <QObject>

class QTimer;

namespace qworldclock {

class TimeEngine : public QObject {
    Q_OBJECT

public:
    static TimeEngine &instance();

    // Prevent copy and move
    TimeEngine(const TimeEngine &) = delete;
    TimeEngine &operator=(const TimeEngine &) = delete;

    void start();
    void stop();
    bool isRunning() const;

    QDateTime currentUtcTime() const;

signals:
    void tick(const QDateTime &utcNow);

private:
    explicit TimeEngine(QObject *parent = nullptr);
    ~TimeEngine() override = default;

    void alignAndStart();
    void onTick();

    QTimer *m_timer{nullptr};
    QTimer *m_alignmentTimer{nullptr};
};

} // namespace qworldclock

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

    void adjustTimeBySeconds(qint64 deltaSeconds);
    void setPreviewTime(const QDateTime &utcTime);
    void resetPreviewTime();
    [[nodiscard]] bool isPreviewActive() const;
    void setPreviewTimeoutMs(int ms);

signals:
    void tick(const QDateTime &utcNow);
    void previewModeChanged(bool active);

private:
    explicit TimeEngine(QObject *parent = nullptr);
    ~TimeEngine() override = default;

    void alignAndStart();
    void onTick();

    QTimer *m_timer{nullptr};
    QTimer *m_alignmentTimer{nullptr};
    QTimer *m_previewResetTimer{nullptr};
    bool m_previewActive{false};
    QDateTime m_previewUtcTime;
    int m_previewTimeoutMs{10000};
};

} // namespace qworldclock

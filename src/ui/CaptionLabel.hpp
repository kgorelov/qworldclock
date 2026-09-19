#pragma once

#include <QDateTime>
#include <QString>
#include <QTimeZone>
#include <QWidget>

namespace qworldclock {

class CaptionLabel : public QWidget {
    Q_OBJECT

public:
    explicit CaptionLabel(QWidget *parent = nullptr);
    ~CaptionLabel() override = default;

    void setCaption(const QString &caption);
    [[nodiscard]] QString caption() const;

    void setTimeZone(const QTimeZone &timeZone);
    [[nodiscard]] QTimeZone timeZone() const;

    void setTime(const QDateTime &utcNow);

    void setFontSize(int size);
    [[nodiscard]] int fontSize() const;

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    [[nodiscard]] QString buildSubtitle() const;

    QString m_caption{QStringLiteral("Local Time")};
    QTimeZone m_timeZone{QTimeZone::systemTimeZone()};
    QDateTime m_currentUtcTime{QDateTime::currentDateTimeUtc()};
    int m_fontSize{0};
};

} // namespace qworldclock

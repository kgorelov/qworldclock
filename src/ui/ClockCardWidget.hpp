#pragma once

#include <QFrame>
#include <QTimeZone>

namespace qworldclock {

class AnalogClockWidget;
class CaptionLabel;

class ClockCardWidget : public QFrame {
    Q_OBJECT

public:
    explicit ClockCardWidget(const QString &id = QString(),
                             const QTimeZone &timeZone = QTimeZone::systemTimeZone(),
                             const QString &caption = QStringLiteral("Local Time"),
                             QWidget *parent = nullptr);
    ~ClockCardWidget() override = default;

    [[nodiscard]] QString clockId() const;
    void setClockId(const QString &id);

    [[nodiscard]] QTimeZone timeZone() const;
    void setTimeZone(const QTimeZone &timeZone);

    [[nodiscard]] QString caption() const;
    void setCaption(const QString &caption);

    [[nodiscard]] int gridRow() const;
    void setGridRow(int row);

    [[nodiscard]] int gridCol() const;
    void setGridCol(int col);

    [[nodiscard]] AnalogClockWidget *analogClock() const;
    [[nodiscard]] CaptionLabel *captionLabel() const;

public slots:
    void setTime(const QDateTime &utcNow);

private:
    void setupUi();

    QString m_clockId;
    int m_gridRow{0};
    int m_gridCol{0};

    AnalogClockWidget *m_analogClock{nullptr};
    CaptionLabel *m_captionLabel{nullptr};
};

} // namespace qworldclock

#pragma once

#include "core/GridModel.hpp"

#include <QFrame>
#include <QPoint>
#include <QTimeZone>

class QPushButton;
class QLabel;
class QDragEnterEvent;
class QDragLeaveEvent;
class QDropEvent;
class QMouseEvent;
class QResizeEvent;

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

    void setEditMode(bool editMode);
    [[nodiscard]] bool isEditMode() const;

    void setCanRemove(bool canRemove);
    [[nodiscard]] bool canRemove() const;

    void setDropHighlighted(bool highlighted);

    void setClockDiameter(int diameter);
    [[nodiscard]] int clockDiameter() const;
    void setResponsive();
    void setFixedClockSize(int clockDiameter);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

signals:
    void requestAdd(const QString &refId, Direction direction);
    void requestRemove(const QString &clockId);
    void clockDropped(const QString &sourceId, const QString &targetId);

public slots:
    void setTime(const QDateTime &utcNow);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUi();
    void setupEditControls();
    void updateButtonPositions();
    void updateCardStyle();

    QString m_clockId;
    int m_gridRow{0};
    int m_gridCol{0};

    AnalogClockWidget *m_analogClock{nullptr};
    CaptionLabel *m_captionLabel{nullptr};

    // Edit controls
    QPushButton *m_btnAddTop{nullptr};
    QPushButton *m_btnAddBottom{nullptr};
    QPushButton *m_btnAddLeft{nullptr};
    QPushButton *m_btnAddRight{nullptr};
    QPushButton *m_btnDelete{nullptr};
    QLabel *m_dragIndicator{nullptr};

    bool m_editMode{false};
    bool m_canRemove{false};
    bool m_dropHighlighted{false};
    QPoint m_dragStartPos;
    int m_clockDiameter{190};
};

} // namespace qworldclock

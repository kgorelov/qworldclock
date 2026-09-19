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

    void setHasCustomWorkingHours(bool custom);
    [[nodiscard]] bool hasCustomWorkingHours() const;

    void setCustomWorkingHours(const WorkingHours &hours);
    [[nodiscard]] WorkingHours customWorkingHours() const;

    void setGlobalWorkingHours(const WorkingHours &hours);
    [[nodiscard]] WorkingHours globalWorkingHours() const;

    [[nodiscard]] WorkingHours effectiveWorkingHours() const;

    void setCaptionFontSize(int size);
    [[nodiscard]] int captionFontSize() const;

    void setGlobalCaptionFontSize(int size);
    [[nodiscard]] int globalCaptionFontSize() const;

    void setHasCustomCaptionFontSize(bool custom);
    [[nodiscard]] bool hasCustomCaptionFontSize() const;

    [[nodiscard]] int effectiveCaptionFontSize() const;

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
    void updateEffectiveWorkingHours();
    void updateEffectiveCaptionFontSize();

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

    bool m_hasCustomWorkingHours{false};
    WorkingHours m_customWorkingHours{8, 0, 18, 0};
    WorkingHours m_globalWorkingHours{8, 0, 18, 0};

    bool m_hasCustomCaptionFontSize{false};
    int m_customCaptionFontSize{0};
    int m_globalCaptionFontSize{0};
};

} // namespace qworldclock

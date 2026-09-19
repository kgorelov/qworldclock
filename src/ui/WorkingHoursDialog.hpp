#pragma once

#include "core/WorkingHours.hpp"

#include <QDialog>
#include <QTimeZone>

class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QTimeEdit;

namespace qworldclock {

enum class WorkingHoursDialogMode {
    Global,
    PerClock
};

class WorkingHoursDialog : public QDialog {
    Q_OBJECT

public:
    explicit WorkingHoursDialog(WorkingHoursDialogMode mode,
                                const WorkingHours &currentHours,
                                const WorkingHours &globalHours,
                                bool hasCustomOverride,
                                const QString &clockCaption = QString(),
                                const QTimeZone &timeZone = QTimeZone::systemTimeZone(),
                                QWidget *parent = nullptr);
    ~WorkingHoursDialog() override = default;

    [[nodiscard]] bool isCustomOverride() const;
    [[nodiscard]] WorkingHours workingHours() const;
    [[nodiscard]] QTime startTime() const;
    [[nodiscard]] QTime endTime() const;

private:
    void setupUi();
    void setPreset(const QTime &start, const QTime &end);

    WorkingHoursDialogMode m_mode;
    WorkingHours m_initialHours;
    WorkingHours m_globalHours;
    bool m_initialHasCustom;
    QString m_clockCaption;
    QTimeZone m_timeZone;

    QCheckBox *m_useGlobalCheckBox{nullptr};
    QWidget *m_timeControlsContainer{nullptr};
    QTimeEdit *m_startTimeEdit{nullptr};
    QTimeEdit *m_endTimeEdit{nullptr};
    QDialogButtonBox *m_buttonBox{nullptr};
};

} // namespace qworldclock

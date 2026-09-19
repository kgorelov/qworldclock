#include "ui/WorkingHoursDialog.hpp"

#include <QCheckBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimeEdit>
#include <QVBoxLayout>

namespace qworldclock {

WorkingHoursDialog::WorkingHoursDialog(WorkingHoursDialogMode mode,
                                       const WorkingHours &currentHours,
                                       const WorkingHours &globalHours,
                                       bool hasCustomOverride,
                                       const QString &clockCaption,
                                       const QTimeZone &timeZone,
                                       QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_initialHours(currentHours)
    , m_globalHours(globalHours)
    , m_initialHasCustom(hasCustomOverride)
    , m_clockCaption(clockCaption)
    , m_timeZone(timeZone) {
    setupUi();
}

bool WorkingHoursDialog::isCustomOverride() const {
    if (m_mode == WorkingHoursDialogMode::Global) {
        return false;
    }
    return m_useGlobalCheckBox && !m_useGlobalCheckBox->isChecked();
}

WorkingHours WorkingHoursDialog::workingHours() const {
    if (isCustomOverride() || m_mode == WorkingHoursDialogMode::Global) {
        return WorkingHours(m_startTimeEdit->time(), m_endTimeEdit->time());
    }
    return m_globalHours;
}

QTime WorkingHoursDialog::startTime() const {
    return workingHours().startTime;
}

QTime WorkingHoursDialog::endTime() const {
    return workingHours().endTime;
}

void WorkingHoursDialog::setPreset(const QTime &start, const QTime &end) {
    if (m_useGlobalCheckBox && m_useGlobalCheckBox->isChecked()) {
        m_useGlobalCheckBox->setChecked(false);
    }
    m_startTimeEdit->setTime(start);
    m_endTimeEdit->setTime(end);
}

void WorkingHoursDialog::setupUi() {
    setModal(true);
    resize(420, m_mode == WorkingHoursDialogMode::PerClock ? 360 : 300);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    if (m_mode == WorkingHoursDialogMode::Global) {
        setWindowTitle(tr("Global Working Hours"));

        auto *descLabel = new QLabel(
            tr("Configure global working hours. During working hours, clock faces remain light. "
               "Outside working hours, clocks with Day/Night shading will darken."),
            this);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet(QStringLiteral("color: #475569; font-size: 13px;"));
        mainLayout->addWidget(descLabel);
    } else {
        setWindowTitle(tr("Working Hours — %1").arg(m_clockCaption));

        const QDateTime localNow = QDateTime::currentDateTimeUtc().toTimeZone(m_timeZone);
        auto *infoLabel = new QLabel(
            tr("<b>Timezone:</b> %1<br><b>Current Local Time:</b> %2")
                .arg(QString::fromUtf8(m_timeZone.id()),
                     localNow.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))),
            this);
        infoLabel->setStyleSheet(QStringLiteral("color: #334155; font-size: 13px; margin-bottom: 4px;"));
        mainLayout->addWidget(infoLabel);

        m_useGlobalCheckBox = new QCheckBox(
            tr("Use global working hours (%1)").arg(m_globalHours.formatRange()),
            this);
        m_useGlobalCheckBox->setChecked(!m_initialHasCustom);
        m_useGlobalCheckBox->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 13px;"));
        mainLayout->addWidget(m_useGlobalCheckBox);
    }

    m_timeControlsContainer = new QWidget(this);
    auto *containerLayout = new QVBoxLayout(m_timeControlsContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(10);

    auto *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setSpacing(10);

    const WorkingHours activeHours = (m_mode == WorkingHoursDialogMode::PerClock && !m_initialHasCustom)
                                         ? m_globalHours
                                         : m_initialHours;

    m_startTimeEdit = new QTimeEdit(activeHours.startTime, m_timeControlsContainer);
    m_startTimeEdit->setDisplayFormat(QStringLiteral("HH:mm"));
    m_startTimeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_endTimeEdit = new QTimeEdit(activeHours.endTime, m_timeControlsContainer);
    m_endTimeEdit->setDisplayFormat(QStringLiteral("HH:mm"));
    m_endTimeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    formLayout->addRow(tr("Start Time:"), m_startTimeEdit);
    formLayout->addRow(tr("End Time:"), m_endTimeEdit);
    containerLayout->addLayout(formLayout);

    // Preset buttons row
    auto *presetLabel = new QLabel(tr("Quick Presets:"), m_timeControlsContainer);
    presetLabel->setStyleSheet(QStringLiteral("color: #64748b; font-size: 12px; font-weight: bold;"));
    containerLayout->addWidget(presetLabel);

    auto *presetLayout = new QHBoxLayout();
    presetLayout->setSpacing(8);

    auto createPresetBtn = [this, presetLayout](const QString &label, const QTime &start, const QTime &end) {
        auto *btn = new QPushButton(label, m_timeControlsContainer);
        btn->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "  background-color: #f1f5f9;"
            "  border: 1px solid #cbd5e1;"
            "  border-radius: 4px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "}"
            "QPushButton:hover { background-color: #e2e8f0; }"
            "QPushButton:pressed { background-color: #cbd5e1; }"));
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, this, [this, start, end]() {
            setPreset(start, end);
        });
        presetLayout->addWidget(btn);
    };

    createPresetBtn(QStringLiteral("08:00 - 18:00"), QTime(8, 0), QTime(18, 0));
    createPresetBtn(QStringLiteral("09:00 - 17:00"), QTime(9, 0), QTime(17, 0));
    createPresetBtn(QStringLiteral("08:00 - 17:00"), QTime(8, 0), QTime(17, 0));
    createPresetBtn(QStringLiteral("07:00 - 19:00"), QTime(7, 0), QTime(19, 0));

    containerLayout->addLayout(presetLayout);
    mainLayout->addWidget(m_timeControlsContainer);

    if (m_mode == WorkingHoursDialogMode::PerClock && m_useGlobalCheckBox) {
        const bool usingGlobal = m_useGlobalCheckBox->isChecked();
        m_timeControlsContainer->setEnabled(!usingGlobal);

        connect(m_useGlobalCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
            m_timeControlsContainer->setEnabled(!checked);
            if (checked) {
                m_startTimeEdit->setTime(m_globalHours.startTime);
                m_endTimeEdit->setTime(m_globalHours.endTime);
            }
        });
    }

    mainLayout->addStretch(1);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(m_buttonBox);
}

} // namespace qworldclock

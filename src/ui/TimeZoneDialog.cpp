#include "ui/TimeZoneDialog.hpp"

#include <QDateTime>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <algorithm>

namespace qworldclock {

TimeZoneDialog::TimeZoneDialog(QWidget *parent)
    : QDialog(parent) {
    setupUi();
    loadTimeZones();
    populateTree();

    // Focus search edit initially
    m_searchEdit->setFocus();
}

void TimeZoneDialog::setupUi() {
    setWindowTitle(tr("Add World Clock — Select Time Zone"));
    resize(720, 520);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(12);

    // 1. Search Box
    auto *searchLayout = new QHBoxLayout();
    auto *searchLabel = new QLabel(tr("Search:"), this);
    searchLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));
    searchLayout->addWidget(searchLabel);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Filter by city, country, offset, or IANA ID (e.g. London, Tokyo, UTC+9)..."));
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &TimeZoneDialog::onFilterChanged);
    searchLayout->addWidget(m_searchEdit, 1);
    mainLayout->addLayout(searchLayout);

    // 2. Tree Widget of Time Zones
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({
        tr("City / Location"),
        tr("Country / Territory"),
        tr("UTC Offset"),
        tr("Current Time"),
        tr("Time Zone ID")
    });
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setSortingEnabled(true);
    m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_treeWidget->header()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_treeWidget->header()->setSectionResizeMode(2, QHeaderView::Interactive);
    m_treeWidget->header()->setSectionResizeMode(3, QHeaderView::Interactive);
    m_treeWidget->header()->setSectionResizeMode(4, QHeaderView::Stretch);

    m_treeWidget->setColumnWidth(0, 150);
    m_treeWidget->setColumnWidth(1, 150);
    m_treeWidget->setColumnWidth(2, 100);
    m_treeWidget->setColumnWidth(3, 110);

    connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &TimeZoneDialog::onItemSelectionChanged);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &TimeZoneDialog::onItemDoubleClicked);
    mainLayout->addWidget(m_treeWidget, 1);

    // 3. Custom Caption Row
    auto *captionLayout = new QHBoxLayout();
    auto *captionLabel = new QLabel(tr("Caption:"), this);
    captionLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));
    captionLayout->addWidget(captionLabel);

    m_captionEdit = new QLineEdit(this);
    m_captionEdit->setPlaceholderText(tr("Custom name underneath the clock widget (defaults to city name)"));
    connect(m_captionEdit, &QLineEdit::textEdited, this, [this]() {
        m_captionManuallyEdited = true;
    });
    captionLayout->addWidget(m_captionEdit, 1);
    mainLayout->addLayout(captionLayout);

    // 4. Buttons
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch(1);

    m_btnCancel = new QPushButton(tr("Cancel"), this);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_btnCancel);

    m_btnAdd = new QPushButton(tr("Add Clock"), this);
    m_btnAdd->setDefault(true);
    m_btnAdd->setEnabled(false);
    m_btnAdd->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: #3b82f6;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 6px 18px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #2563eb; }"
        "QPushButton:disabled { background-color: #cbd5e1; color: #94a3b8; }"));
    connect(m_btnAdd, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_btnAdd);

    mainLayout->addLayout(btnLayout);
}

void TimeZoneDialog::loadTimeZones() {
    m_entries.clear();
    const auto availableIds = QTimeZone::availableTimeZoneIds();
    const QDateTime nowUtc = QDateTime::currentDateTimeUtc();

    for (const auto &tzId : availableIds) {
        QTimeZone tz(tzId);
        if (!tz.isValid()) {
            continue;
        }

        const QString idStr = QString::fromUtf8(tzId);
        // Skip purely technical system/posix zones like Etc/GMT+... unless useful
        if (idStr.startsWith(QStringLiteral("Etc/GMT"))) {
            continue;
        }

        QString city = idStr.section('/', -1).replace('_', ' ');
        QString region = idStr.section('/', 0, 0);
        QString territory = QLocale::territoryToString(tz.territory());
        if (territory.isEmpty() || territory == QStringLiteral("World")) {
            territory = region;
        }

        const int offsetSec = tz.offsetFromUtc(nowUtc);
        const int hours = offsetSec / 3600;
        const int mins = qAbs((offsetSec % 3600) / 60);

        QString offsetStr = QStringLiteral("UTC%1%2:%3")
                                .arg(hours >= 0 ? "+" : "-")
                                .arg(qAbs(hours), 2, 10, QLatin1Char('0'))
                                .arg(mins, 2, 10, QLatin1Char('0'));

        const QDateTime localTime = nowUtc.toTimeZone(tz);
        QString timeStr = localTime.toString(QStringLiteral("hh:mm"));
        if (tz.isDaylightTime(nowUtc)) {
            timeStr += QStringLiteral(" (DST)");
        }

        m_entries.append({
            tzId,
            city,
            region,
            territory,
            offsetStr,
            offsetSec,
            timeStr
        });
    }

    // Sort alphabetically by city name
    std::sort(m_entries.begin(), m_entries.end(), [](const TimeZoneEntry &a, const TimeZoneEntry &b) {
        return a.city.localeAwareCompare(b.city) < 0;
    });
}

void TimeZoneDialog::populateTree() {
    m_treeWidget->setUpdatesEnabled(false);
    m_treeWidget->clear();

    for (const auto &entry : m_entries) {
        auto *item = new QTreeWidgetItem(m_treeWidget);
        item->setText(0, entry.city);
        item->setText(1, entry.territory);
        item->setText(2, entry.offsetStr);
        item->setText(3, entry.currentTimeStr);
        item->setText(4, QString::fromUtf8(entry.id));
        item->setData(0, Qt::UserRole, entry.id);
    }

    m_treeWidget->setUpdatesEnabled(true);

    // Select the first item by default if available
    if (m_treeWidget->topLevelItemCount() > 0) {
        m_treeWidget->setCurrentItem(m_treeWidget->topLevelItem(0));
    }
}

void TimeZoneDialog::onFilterChanged(const QString &text) {
    const QString trimmed = text.trimmed();
    const bool empty = trimmed.isEmpty();

    m_treeWidget->setUpdatesEnabled(false);
    QTreeWidgetItem *firstVisible = nullptr;

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        auto *item = m_treeWidget->topLevelItem(i);
        if (empty) {
            item->setHidden(false);
            if (!firstVisible) {
                firstVisible = item;
            }
        } else {
            const bool match = item->text(0).contains(trimmed, Qt::CaseInsensitive) ||
                               item->text(1).contains(trimmed, Qt::CaseInsensitive) ||
                               item->text(2).contains(trimmed, Qt::CaseInsensitive) ||
                               item->text(4).contains(trimmed, Qt::CaseInsensitive);
            item->setHidden(!match);
            if (match && !firstVisible) {
                firstVisible = item;
            }
        }
    }

    if (firstVisible) {
        m_treeWidget->setCurrentItem(firstVisible);
    }

    m_treeWidget->setUpdatesEnabled(true);
}

void TimeZoneDialog::onItemSelectionChanged() {
    auto *item = m_treeWidget->currentItem();
    if (!item) {
        m_btnAdd->setEnabled(false);
        return;
    }

    m_btnAdd->setEnabled(true);

    if (!m_captionManuallyEdited) {
        m_captionEdit->setText(item->text(0));
    }
}

void TimeZoneDialog::onItemDoubleClicked(QTreeWidgetItem * /*item*/, int /*column*/) {
    if (m_btnAdd->isEnabled()) {
        accept();
    }
}

QTimeZone TimeZoneDialog::selectedTimeZone() const {
    auto *item = m_treeWidget->currentItem();
    if (!item) {
        return QTimeZone::systemTimeZone();
    }
    const QByteArray tzId = item->data(0, Qt::UserRole).toByteArray();
    return QTimeZone(tzId);
}

QString TimeZoneDialog::selectedCaption() const {
    const QString text = m_captionEdit->text().trimmed();
    if (!text.isEmpty()) {
        return text;
    }
    auto *item = m_treeWidget->currentItem();
    return item ? item->text(0) : tr("World Clock");
}

} // namespace qworldclock

#pragma once

#include <QDialog>
#include <QList>
#include <QTimeZone>

class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

namespace qworldclock {

struct TimeZoneEntry {
    QByteArray id;
    QString city;
    QString region;
    QString territory;
    QString offsetStr;
    int offsetSeconds{0};
    QString currentTimeStr;
};

class TimeZoneDialog : public QDialog {
    Q_OBJECT

public:
    explicit TimeZoneDialog(QWidget *parent = nullptr);
    ~TimeZoneDialog() override = default;

    [[nodiscard]] QTimeZone selectedTimeZone() const;
    [[nodiscard]] QString selectedCaption() const;

private slots:
    void onFilterChanged(const QString &text);
    void onItemSelectionChanged();
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void setupUi();
    void loadTimeZones();
    void populateTree();

    QLineEdit *m_searchEdit{nullptr};
    QLineEdit *m_captionEdit{nullptr};
    QTreeWidget *m_treeWidget{nullptr};
    QPushButton *m_btnAdd{nullptr};
    QPushButton *m_btnCancel{nullptr};

    QList<TimeZoneEntry> m_entries;
    bool m_captionManuallyEdited{false};
};

} // namespace qworldclock

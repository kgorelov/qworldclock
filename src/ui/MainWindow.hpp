#pragma once

#include <QMainWindow>
#include <QActionGroup>

class QLabel;
class QBoxLayout;
class QContextMenuEvent;

namespace qworldclock {

class ClockCardWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void setupUi();
    void setupMenus();
    void setupStatusBar();
    void updateLayoutAlignment(Qt::Alignment alignment);

    // Menu and Action handlers
    void onToggleEditMode(bool checked);
    void onAlignmentChanged(QAction *action);
    void onSizingModeChanged(QAction *action);
    void onToggleMenuBar(bool checked);
    void onToggleStatusBar(bool checked);
    void onToggleSeconds(bool checked);
    void onToggleDayNight(bool checked);

    // UI elements & actions
    QLabel *m_statusLabel{nullptr};
    QBoxLayout *m_centralContainerLayout{nullptr};
    ClockCardWidget *m_primaryClock{nullptr};

    QAction *m_editModeAction{nullptr};
    QAction *m_toggleMenuBarAction{nullptr};
    QAction *m_toggleStatusBarAction{nullptr};
    QAction *m_toggleSecondsAction{nullptr};
    QAction *m_toggleDayNightAction{nullptr};
    QAction *m_exitAction{nullptr};
    QActionGroup *m_alignmentGroup{nullptr};
    QActionGroup *m_sizingGroup{nullptr};
};

} // namespace qworldclock

#pragma once

#include "core/GridModel.hpp"

#include <QMainWindow>
#include <QActionGroup>

class QFrame;
class QLabel;
class QMenu;
class QContextMenuEvent;

namespace qworldclock {

class GridModel;
class ClockGridPanel;

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

    // Menu and Action handlers
    void onToggleEditMode(bool checked);
    void onAlignmentChanged(QAction *action);
    void onSizingModeChanged(QAction *action);
    void onClockSizeChanged(QAction *action);
    void onCustomClockSize();
    void onToggleMenuBar(bool checked);
    void onToggleStatusBar(bool checked);
    void onToggleSeconds(bool checked);
    void onToggleDayNight(bool checked);
    void onAddClockRequested(const QString &refId, Direction direction);

    // Core data & UI components
    GridModel *m_gridModel{nullptr};
    ClockGridPanel *m_gridPanel{nullptr};
    QFrame *m_editBanner{nullptr};

    QLabel *m_statusLabel{nullptr};
    QAction *m_editModeAction{nullptr};
    QAction *m_responsiveModeAction{nullptr};
    QAction *m_fixedModeAction{nullptr};
    QAction *m_toggleMenuBarAction{nullptr};
    QAction *m_toggleStatusBarAction{nullptr};
    QAction *m_toggleSecondsAction{nullptr};
    QAction *m_toggleDayNightAction{nullptr};
    QAction *m_exitAction{nullptr};
    QActionGroup *m_alignmentGroup{nullptr};
    QActionGroup *m_sizingGroup{nullptr};
    QActionGroup *m_clockSizeGroup{nullptr};
    QMenu *m_clockSizeMenu{nullptr};

    bool m_showSeconds{true};
    bool m_showDayNight{true};
};

} // namespace qworldclock

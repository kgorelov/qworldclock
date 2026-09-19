#pragma once

#include "core/ConfigManager.hpp"
#include "core/GridModel.hpp"

#include <QMainWindow>
#include <QActionGroup>

class QCloseEvent;
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
    explicit MainWindow(const QString &configFilePath, QWidget *parent = nullptr);
    ~MainWindow() override = default;

    void saveConfig();
    void loadConfig();

    [[nodiscard]] ConfigManager &configManager();
    [[nodiscard]] const ConfigManager &configManager() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

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
    void onConfigureGlobalWorkingHours();
    void onConfigureClockWorkingHours(const QString &clockId);
    void onResetClockWorkingHours(const QString &clockId);
    void onSetGlobalWorkingHours(const WorkingHours &hours);
    void onSetGlobalStartTime(const QTime &time);
    void onSetGlobalEndTime(const QTime &time);

    QMenu *createWorkingHoursSubmenu(QWidget *parentMenu);

    // Core data & UI components
    ConfigManager m_configManager;
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
    QMenu *m_workingHoursMenu{nullptr};

    bool m_showSeconds{true};
    bool m_showDayNight{true};
    WorkingHours m_workingHours{8, 0, 18, 0};
    bool m_isLoadingConfig{false};
};

} // namespace qworldclock

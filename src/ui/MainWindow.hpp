#pragma once

#include <QMainWindow>
#include <QActionGroup>

class QLabel;
class QContextMenuEvent;

namespace qworldclock {

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
    void onToggleMenuBar(bool checked);
    void onToggleStatusBar(bool checked);

    // UI elements & actions
    QLabel *m_statusLabel{nullptr};
    QAction *m_editModeAction{nullptr};
    QAction *m_toggleMenuBarAction{nullptr};
    QAction *m_toggleStatusBarAction{nullptr};
    QAction *m_exitAction{nullptr};
    QActionGroup *m_alignmentGroup{nullptr};
    QActionGroup *m_sizingGroup{nullptr};
};

} // namespace qworldclock

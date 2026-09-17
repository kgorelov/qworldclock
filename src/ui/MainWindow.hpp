#pragma once

#include <QMainWindow>
#include <QActionGroup>

class QLabel;

namespace qworldclock {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private:
    void setupUi();
    void setupMenus();
    void setupToolBar();
    void setupStatusBar();

    // Menu and Action handlers
    void onToggleEditMode(bool checked);
    void onAlignmentChanged(QAction *action);
    void onSizingModeChanged(QAction *action);

    // UI elements
    QLabel *m_statusLabel{nullptr};
    QAction *m_editModeAction{nullptr};
    QActionGroup *m_alignmentGroup{nullptr};
    QActionGroup *m_sizingGroup{nullptr};
};

} // namespace qworldclock

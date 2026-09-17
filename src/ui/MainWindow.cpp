#include "ui/MainWindow.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QContextMenuEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>

namespace qworldclock {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
}

void MainWindow::setupUi() {
    setWindowTitle(QStringLiteral("QWorldClock"));
    resize(800, 600);

    setupMenus();
    setupStatusBar();

    // Central placeholder widget for Phase 1
    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    layout->setAlignment(Qt::AlignCenter);

    auto *welcomeLabel = new QLabel(
        QStringLiteral("<h2>QWorldClock</h2>"
                       "<p>World Clock application initialized successfully.</p>"
                       "<p><i>Clean interface: right-click anywhere for menu.</i></p>"
                       "<p><small>Shortcuts: <b>Ctrl+M</b> toggle menu | <b>Ctrl+E</b> edit layout</small></p>"),
        centralWidget);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(welcomeLabel);

    setCentralWidget(centralWidget);
}

void MainWindow::setupMenus() {
    auto *menuBar = this->menuBar();

    // File Menu
    auto *fileMenu = menuBar->addMenu(tr("&File"));
    m_exitAction = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    m_exitAction->setShortcut(QKeySequence::Quit);
    addAction(m_exitAction);

    // Edit Menu
    auto *editMenu = menuBar->addMenu(tr("&Edit"));
    m_editModeAction = editMenu->addAction(tr("&Edit Layout"), this, &MainWindow::onToggleEditMode);
    m_editModeAction->setCheckable(true);
    m_editModeAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+E")));
    addAction(m_editModeAction);

    // View Menu
    auto *viewMenu = menuBar->addMenu(tr("&View"));

    // Alignment Submenu
    auto *alignmentMenu = viewMenu->addMenu(tr("&Clock Alignment"));
    m_alignmentGroup = new QActionGroup(this);
    m_alignmentGroup->setExclusive(true);

    auto *alignLeft = alignmentMenu->addAction(tr("&Left"));
    alignLeft->setCheckable(true);
    alignLeft->setData(QStringLiteral("left"));
    alignLeft->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+L")));
    m_alignmentGroup->addAction(alignLeft);
    addAction(alignLeft);

    auto *alignCenter = alignmentMenu->addAction(tr("&Center"));
    alignCenter->setCheckable(true);
    alignCenter->setChecked(true);
    alignCenter->setData(QStringLiteral("center"));
    alignCenter->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+C")));
    m_alignmentGroup->addAction(alignCenter);
    addAction(alignCenter);

    auto *alignRight = alignmentMenu->addAction(tr("&Right"));
    alignRight->setCheckable(true);
    alignRight->setData(QStringLiteral("right"));
    alignRight->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+R")));
    m_alignmentGroup->addAction(alignRight);
    addAction(alignRight);

    connect(m_alignmentGroup, &QActionGroup::triggered, this, &MainWindow::onAlignmentChanged);

    // Sizing Submenu
    auto *sizingMenu = viewMenu->addMenu(tr("&Sizing Mode"));
    m_sizingGroup = new QActionGroup(this);
    m_sizingGroup->setExclusive(true);

    auto *responsiveMode = sizingMenu->addAction(tr("&Responsive (Auto-Scale)"));
    responsiveMode->setCheckable(true);
    responsiveMode->setChecked(true);
    responsiveMode->setData(QStringLiteral("responsive"));
    m_sizingGroup->addAction(responsiveMode);

    auto *fixedMode = sizingMenu->addAction(tr("&Fixed Size"));
    fixedMode->setCheckable(true);
    fixedMode->setData(QStringLiteral("fixed"));
    m_sizingGroup->addAction(fixedMode);

    connect(m_sizingGroup, &QActionGroup::triggered, this, &MainWindow::onSizingModeChanged);

    viewMenu->addSeparator();

    // Toggle Menu Bar Action
    m_toggleMenuBarAction = viewMenu->addAction(tr("Show &Menu Bar"), this, &MainWindow::onToggleMenuBar);
    m_toggleMenuBarAction->setCheckable(true);
    m_toggleMenuBarAction->setChecked(true);
    m_toggleMenuBarAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+M")));
    addAction(m_toggleMenuBarAction);

    // Toggle Status Bar Action
    m_toggleStatusBarAction = viewMenu->addAction(tr("Show &Status Bar"), this, &MainWindow::onToggleStatusBar);
    m_toggleStatusBarAction->setCheckable(true);
    m_toggleStatusBarAction->setChecked(true);
    addAction(m_toggleStatusBarAction);
}

void MainWindow::setupStatusBar() {
    auto *statusBar = this->statusBar();
    m_statusLabel = new QLabel(tr("Ready"), this);
    statusBar->addWidget(m_statusLabel);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event) {
    QMenu contextMenu(this);

    contextMenu.addAction(m_editModeAction);
    contextMenu.addSeparator();

    auto *alignMenu = contextMenu.addMenu(tr("Clock Alignment"));
    for (auto *action : m_alignmentGroup->actions()) {
        alignMenu->addAction(action);
    }

    auto *sizingMenu = contextMenu.addMenu(tr("Sizing Mode"));
    for (auto *action : m_sizingGroup->actions()) {
        sizingMenu->addAction(action);
    }

    contextMenu.addSeparator();
    contextMenu.addAction(m_toggleMenuBarAction);
    contextMenu.addAction(m_toggleStatusBarAction);
    contextMenu.addSeparator();
    contextMenu.addAction(m_exitAction);

    contextMenu.exec(event->globalPos());
}

void MainWindow::onToggleEditMode(bool checked) {
    if (m_statusLabel) {
        m_statusLabel->setText(checked ? tr("Edit Mode: Active") : tr("Ready"));
    }
}

void MainWindow::onAlignmentChanged(QAction *action) {
    if (!action) {
        return;
    }
    const QString alignment = action->data().toString();
    if (m_statusLabel) {
        m_statusLabel->setText(tr("Alignment set to: %1").arg(alignment));
    }
}

void MainWindow::onSizingModeChanged(QAction *action) {
    if (!action) {
        return;
    }
    const QString mode = action->data().toString();
    if (m_statusLabel) {
        m_statusLabel->setText(tr("Sizing mode set to: %1").arg(mode));
    }
}

void MainWindow::onToggleMenuBar(bool checked) {
    menuBar()->setVisible(checked);
    m_toggleMenuBarAction->setChecked(checked);
    if (m_statusLabel) {
        m_statusLabel->setText(checked ? tr("Menu bar shown") : tr("Menu bar hidden (press Ctrl+M or right-click to restore)"));
    }
}

void MainWindow::onToggleStatusBar(bool checked) {
    statusBar()->setVisible(checked);
    m_toggleStatusBarAction->setChecked(checked);
}

} // namespace qworldclock

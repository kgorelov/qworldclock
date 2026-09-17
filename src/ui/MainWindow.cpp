#include "ui/MainWindow.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
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
    setupToolBar();
    setupStatusBar();

    // Central placeholder widget for Phase 1
    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    layout->setAlignment(Qt::AlignCenter);

    auto *welcomeLabel = new QLabel(
        QStringLiteral("<h2>QWorldClock</h2>"
                       "<p>World Clock application initialized successfully.</p>"
                       "<p><i>Phase 1: Environment &amp; Project Scaffolding</i></p>"),
        centralWidget);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(welcomeLabel);

    setCentralWidget(centralWidget);
}

void MainWindow::setupMenus() {
    auto *menuBar = this->menuBar();

    // File Menu
    auto *fileMenu = menuBar->addMenu(tr("&File"));
    auto *exitAction = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAction->setShortcut(QKeySequence::Quit);

    // Edit Menu
    auto *editMenu = menuBar->addMenu(tr("&Edit"));
    m_editModeAction = editMenu->addAction(tr("&Edit Layout"), this, &MainWindow::onToggleEditMode);
    m_editModeAction->setCheckable(true);
    m_editModeAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+E")));

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

    auto *alignCenter = alignmentMenu->addAction(tr("&Center"));
    alignCenter->setCheckable(true);
    alignCenter->setChecked(true);
    alignCenter->setData(QStringLiteral("center"));
    alignCenter->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+C")));
    m_alignmentGroup->addAction(alignCenter);

    auto *alignRight = alignmentMenu->addAction(tr("&Right"));
    alignRight->setCheckable(true);
    alignRight->setData(QStringLiteral("right"));
    alignRight->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+R")));
    m_alignmentGroup->addAction(alignRight);

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
}

void MainWindow::setupToolBar() {
    auto *toolBar = addToolBar(tr("Main Toolbar"));
    toolBar->setMovable(false);

    toolBar->addAction(m_editModeAction);
    toolBar->addSeparator();

    // Add alignment quick actions to toolbar
    for (auto *action : m_alignmentGroup->actions()) {
        toolBar->addAction(action);
    }
}

void MainWindow::setupStatusBar() {
    auto *statusBar = this->statusBar();
    m_statusLabel = new QLabel(tr("Ready"), this);
    statusBar->addWidget(m_statusLabel);
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

} // namespace qworldclock

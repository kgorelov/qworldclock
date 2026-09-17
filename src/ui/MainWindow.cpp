#include "ui/MainWindow.hpp"
#include "core/TimeEngine.hpp"
#include "ui/AnalogClockWidget.hpp"
#include "ui/ClockCardWidget.hpp"
#include "ui/ClockGridPanel.hpp"
#include "ui/TimeZoneDialog.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QContextMenuEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QVBoxLayout>

namespace qworldclock {

MainWindow::MainWindow(QWidget *parent)
    : MainWindow(QString(), parent) {
}

MainWindow::MainWindow(const QString &configFilePath, QWidget *parent)
    : QMainWindow(parent)
    , m_configManager(configFilePath) {
    setupUi();
    loadConfig();

    connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::saveConfig);
}

void MainWindow::setupUi() {
    setWindowTitle(QStringLiteral("QWorldClock"));
    resize(900, 650);

    setupMenus();
    setupStatusBar();

    // Start central time engine
    TimeEngine::instance().start();

    // Initialize 2D Grid Model
    m_gridModel = new GridModel(this);
    connect(m_gridModel, &GridModel::layoutChanged, this, &MainWindow::saveConfig);

    // Initialize Clock Grid Panel
    m_gridPanel = new ClockGridPanel(m_gridModel, this);
    connect(m_gridPanel, &ClockGridPanel::clockCountChanged, this, [this](int count) {
        if (m_statusLabel) {
            m_statusLabel->setText(tr("%n clock(s) active", "", count));
        }
    });
    connect(m_gridPanel, &ClockGridPanel::requestAddClock, this, &MainWindow::onAddClockRequested);

    // Host grid and edit banner inside a vertical main layout
    auto *mainContainer = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Edit Mode Banner (hidden by default)
    m_editBanner = new QFrame(mainContainer);
    m_editBanner->setObjectName(QStringLiteral("EditBanner"));
    m_editBanner->setStyleSheet(QStringLiteral(
        "#EditBanner {"
        "  background-color: #1e293b;"
        "  border-bottom: 1px solid #334155;"
        "}"));
    auto *bannerLayout = new QHBoxLayout(m_editBanner);
    bannerLayout->setContentsMargins(16, 6, 16, 6);

    auto *bannerText = new QLabel(
        tr("<b>Edit Mode</b> — Click <b>+</b> to add adjacent clocks, <b>✕</b> to remove, or drag cards to reorder."),
        m_editBanner);
    bannerText->setStyleSheet(QStringLiteral("color: #f8fafc; font-size: 13px;"));
    bannerLayout->addWidget(bannerText);
    bannerLayout->addStretch(1);

    auto *doneBtn = new QPushButton(tr("Done Editing"), m_editBanner);
    doneBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: #3b82f6;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 5px 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #2563eb; }"));
    doneBtn->setCursor(Qt::PointingHandCursor);
    connect(doneBtn, &QPushButton::clicked, this, [this]() {
        m_editModeAction->setChecked(false);
        onToggleEditMode(false);
    });
    bannerLayout->addWidget(doneBtn);

    m_editBanner->setVisible(false);
    mainLayout->addWidget(m_editBanner);

    // Scroll area hosting grid panel
    auto *scrollArea = new QScrollArea(mainContainer);
    scrollArea->setWidget(m_gridPanel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    mainLayout->addWidget(scrollArea, 1);

    setCentralWidget(mainContainer);
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

    m_responsiveModeAction = sizingMenu->addAction(tr("&Responsive (Auto-Scale)"));
    m_responsiveModeAction->setCheckable(true);
    m_responsiveModeAction->setChecked(true);
    m_responsiveModeAction->setData(QStringLiteral("responsive"));
    m_sizingGroup->addAction(m_responsiveModeAction);

    m_fixedModeAction = sizingMenu->addAction(tr("&Fixed Size"));
    m_fixedModeAction->setCheckable(true);
    m_fixedModeAction->setData(QStringLiteral("fixed"));
    m_sizingGroup->addAction(m_fixedModeAction);

    connect(m_sizingGroup, &QActionGroup::triggered, this, &MainWindow::onSizingModeChanged);

    // Clock Size Submenu (for Fixed Size Mode)
    m_clockSizeMenu = viewMenu->addMenu(tr("Clock &Size"));
    m_clockSizeGroup = new QActionGroup(this);
    m_clockSizeGroup->setExclusive(true);

    auto *sizeSmall = m_clockSizeMenu->addAction(tr("&Small (140 px)"));
    sizeSmall->setCheckable(true);
    sizeSmall->setData(140);
    m_clockSizeGroup->addAction(sizeSmall);

    auto *sizeMedium = m_clockSizeMenu->addAction(tr("&Medium (190 px)"));
    sizeMedium->setCheckable(true);
    sizeMedium->setChecked(true);
    sizeMedium->setData(190);
    m_clockSizeGroup->addAction(sizeMedium);

    auto *sizeLarge = m_clockSizeMenu->addAction(tr("&Large (250 px)"));
    sizeLarge->setCheckable(true);
    sizeLarge->setData(250);
    m_clockSizeGroup->addAction(sizeLarge);

    auto *sizeXLarge = m_clockSizeMenu->addAction(tr("&Extra Large (320 px)"));
    sizeXLarge->setCheckable(true);
    sizeXLarge->setData(320);
    m_clockSizeGroup->addAction(sizeXLarge);

    connect(m_clockSizeGroup, &QActionGroup::triggered, this, &MainWindow::onClockSizeChanged);

    m_clockSizeMenu->addSeparator();
    m_clockSizeMenu->addAction(tr("&Custom Size..."), this, &MainWindow::onCustomClockSize);

    viewMenu->addSeparator();

    // Seconds and Day/Night Toggles
    m_toggleSecondsAction = viewMenu->addAction(tr("Show &Seconds Hand"), this, &MainWindow::onToggleSeconds);
    m_toggleSecondsAction->setCheckable(true);
    m_toggleSecondsAction->setChecked(true);

    m_toggleDayNightAction = viewMenu->addAction(tr("Show &Day/Night Shading"), this, &MainWindow::onToggleDayNight);
    m_toggleDayNightAction->setCheckable(true);
    m_toggleDayNightAction->setChecked(true);

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
    m_statusLabel = new QLabel(tr("1 clock active"), this);
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

    auto *sizeMenu = contextMenu.addMenu(tr("Clock Size"));
    for (auto *action : m_clockSizeGroup->actions()) {
        sizeMenu->addAction(action);
    }
    sizeMenu->addSeparator();
    sizeMenu->addAction(tr("Custom Size..."), this, &MainWindow::onCustomClockSize);

    contextMenu.addSeparator();
    contextMenu.addAction(m_toggleSecondsAction);
    contextMenu.addAction(m_toggleDayNightAction);

    contextMenu.addSeparator();
    contextMenu.addAction(m_toggleMenuBarAction);
    contextMenu.addAction(m_toggleStatusBarAction);
    contextMenu.addSeparator();
    contextMenu.addAction(m_exitAction);

    contextMenu.exec(event->globalPos());
}

void MainWindow::onToggleEditMode(bool checked) {
    if (m_editBanner) {
        m_editBanner->setVisible(checked);
    }
    if (m_gridPanel) {
        m_gridPanel->setEditMode(checked);
    }
    if (m_statusLabel) {
        m_statusLabel->setText(checked ? tr("Edit Mode: Active") : tr("%n clock(s) active", "", m_gridModel ? m_gridModel->count() : 1));
    }
}

void MainWindow::onAlignmentChanged(QAction *action) {
    if (!action) {
        return;
    }
    const QString alignment = action->data().toString();
    Qt::Alignment alignFlag = Qt::AlignCenter;
    if (alignment == QStringLiteral("left")) {
        alignFlag = Qt::AlignLeft;
    } else if (alignment == QStringLiteral("right")) {
        alignFlag = Qt::AlignRight;
    } else {
        alignFlag = Qt::AlignCenter;
    }

    if (m_gridPanel) {
        m_gridPanel->setGridAlignment(alignFlag);
    }

    if (m_statusLabel) {
        m_statusLabel->setText(tr("Alignment set to: %1").arg(alignment));
    }
    saveConfig();
}

void MainWindow::onSizingModeChanged(QAction *action) {
    if (!action || !m_gridPanel) {
        return;
    }
    const QString mode = action->data().toString();
    if (mode == QStringLiteral("fixed")) {
        m_gridPanel->setSizingMode(SizingMode::Fixed);
        if (m_statusLabel) {
            m_statusLabel->setText(tr("Sizing mode: Fixed (%1 px)").arg(m_gridPanel->fixedClockSize()));
        }
    } else {
        m_gridPanel->setSizingMode(SizingMode::Responsive);
        if (m_statusLabel) {
            m_statusLabel->setText(tr("Sizing mode: Responsive (Auto-Scale)"));
        }
    }
    saveConfig();
}

void MainWindow::onClockSizeChanged(QAction *action) {
    if (!action || !m_gridPanel) {
        return;
    }
    const int size = action->data().toInt();
    m_gridPanel->setFixedClockSize(size);
    if (m_fixedModeAction) {
        m_fixedModeAction->setChecked(true);
    }
    m_gridPanel->setSizingMode(SizingMode::Fixed);
    if (m_statusLabel) {
        m_statusLabel->setText(tr("Clock size set to %1 px (Fixed Mode)").arg(size));
    }
    saveConfig();
}

void MainWindow::onCustomClockSize() {
    if (!m_gridPanel) {
        return;
    }
    bool ok = false;
    const int currentSize = m_gridPanel->fixedClockSize();
    const int size = QInputDialog::getInt(
        this,
        tr("Custom Clock Size"),
        tr("Clock diameter in pixels (100 - 500):"),
        currentSize,
        100,
        500,
        10,
        &ok);

    if (ok) {
        m_gridPanel->setFixedClockSize(size);
        if (m_fixedModeAction) {
            m_fixedModeAction->setChecked(true);
        }
        m_gridPanel->setSizingMode(SizingMode::Fixed);
        if (m_statusLabel) {
            m_statusLabel->setText(tr("Clock size set to %1 px (Fixed Mode)").arg(size));
        }
        saveConfig();
    }
}

void MainWindow::onToggleMenuBar(bool checked) {
    menuBar()->setVisible(checked);
    m_toggleMenuBarAction->setChecked(checked);
    if (m_statusLabel) {
        m_statusLabel->setText(checked ? tr("Menu bar shown") : tr("Menu bar hidden (press Ctrl+M or right-click to restore)"));
    }
    saveConfig();
}

void MainWindow::onToggleStatusBar(bool checked) {
    statusBar()->setVisible(checked);
    m_toggleStatusBarAction->setChecked(checked);
    saveConfig();
}

void MainWindow::onToggleSeconds(bool checked) {
    m_showSeconds = checked;
    if (m_gridModel && m_gridPanel) {
        for (const auto &item : m_gridModel->clocks()) {
            auto *card = m_gridPanel->cardWidget(item.id);
            if (card && card->analogClock()) {
                card->analogClock()->setShowSeconds(checked);
            }
        }
    }
    saveConfig();
}

void MainWindow::onToggleDayNight(bool checked) {
    m_showDayNight = checked;
    if (m_gridModel && m_gridPanel) {
        for (const auto &item : m_gridModel->clocks()) {
            auto *card = m_gridPanel->cardWidget(item.id);
            if (card && card->analogClock()) {
                card->analogClock()->setShowDayNightShading(checked);
            }
        }
    }
    saveConfig();
}

void MainWindow::onAddClockRequested(const QString &refId, Direction direction) {
    TimeZoneDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        const QTimeZone tz = dialog.selectedTimeZone();
        const QString caption = dialog.selectedCaption();
        m_gridPanel->addClockRelative(refId, direction, tz, caption);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveConfig();
    QMainWindow::closeEvent(event);
}

void MainWindow::loadConfig() {
    m_isLoadingConfig = true;

    const AppConfig cfg = m_configManager.load();

    // 1. Clocks
    if (m_gridModel) {
        m_gridModel->blockSignals(true);
        m_gridModel->clear();
        if (cfg.clocks.isEmpty()) {
            m_gridModel->addClock({
                QStringLiteral("clock-local"),
                QTimeZone::systemTimeZone(),
                tr("Local Time"),
                0,
                0
            });
        } else {
            for (const auto &item : cfg.clocks) {
                m_gridModel->addClock(item);
            }
        }
        m_gridModel->blockSignals(false);
    }

    // 2. Sizing & Alignment on Grid Panel
    if (m_gridPanel) {
        m_gridPanel->setFixedClockSize(cfg.fixedClockSize);
        m_gridPanel->setSizingMode(ConfigManager::stringToSizingMode(cfg.sizingMode));
        m_gridPanel->setGridAlignment(ConfigManager::stringToAlignment(cfg.alignment));
        m_gridPanel->refreshLayout();
    }

    // 3. Update Menu Actions / UI State
    if (m_alignmentGroup) {
        for (auto *action : m_alignmentGroup->actions()) {
            if (action->data().toString() == cfg.alignment) {
                action->setChecked(true);
                break;
            }
        }
    }

    if (cfg.sizingMode == QStringLiteral("fixed")) {
        if (m_fixedModeAction) {
            m_fixedModeAction->setChecked(true);
        }
    } else {
        if (m_responsiveModeAction) {
            m_responsiveModeAction->setChecked(true);
        }
    }

    if (m_clockSizeGroup) {
        for (auto *action : m_clockSizeGroup->actions()) {
            if (action->data().toInt() == cfg.fixedClockSize) {
                action->setChecked(true);
                break;
            }
        }
    }

    m_showSeconds = cfg.showSeconds;
    if (m_toggleSecondsAction) {
        m_toggleSecondsAction->setChecked(cfg.showSeconds);
    }
    onToggleSeconds(cfg.showSeconds);

    m_showDayNight = cfg.showDayNight;
    if (m_toggleDayNightAction) {
        m_toggleDayNightAction->setChecked(cfg.showDayNight);
    }
    onToggleDayNight(cfg.showDayNight);

    if (m_toggleMenuBarAction) {
        m_toggleMenuBarAction->setChecked(cfg.showMenuBar);
    }
    menuBar()->setVisible(cfg.showMenuBar);

    if (m_toggleStatusBarAction) {
        m_toggleStatusBarAction->setChecked(cfg.showStatusBar);
    }
    statusBar()->setVisible(cfg.showStatusBar);

    // Window Geometry
    if (cfg.window.width >= 200 && cfg.window.height >= 200) {
        resize(cfg.window.width, cfg.window.height);
    }
    if (cfg.window.x >= 0 && cfg.window.y >= 0) {
        move(cfg.window.x, cfg.window.y);
    }
    if (cfg.window.maximized) {
        showMaximized();
    }

    m_isLoadingConfig = false;
}

void MainWindow::saveConfig() {
    if (m_isLoadingConfig || !m_gridModel || !m_gridPanel) {
        return;
    }

    AppConfig cfg;
    cfg.version = QStringLiteral("1.0.0");
    cfg.sizingMode = ConfigManager::sizingModeToString(m_gridPanel->sizingMode());
    cfg.fixedClockSize = m_gridPanel->fixedClockSize();
    cfg.alignment = ConfigManager::alignmentToString(m_gridPanel->gridAlignment());
    cfg.showMenuBar = menuBar()->isVisible();
    cfg.showStatusBar = statusBar()->isVisible();
    cfg.showSeconds = m_showSeconds;
    cfg.showDayNight = m_showDayNight;

    cfg.window.maximized = isMaximized();
    if (!isMaximized()) {
        cfg.window.width = width();
        cfg.window.height = height();
        cfg.window.x = pos().x();
        cfg.window.y = pos().y();
    } else {
        const QRect norm = normalGeometry();
        cfg.window.width = norm.width();
        cfg.window.height = norm.height();
        cfg.window.x = norm.x();
        cfg.window.y = norm.y();
    }

    const auto &clocksVec = m_gridModel->clocks();
    cfg.clocks.clear();
    for (const auto &item : clocksVec) {
        cfg.clocks.append(item);
    }

    m_configManager.save(cfg);
}

ConfigManager &MainWindow::configManager() {
    return m_configManager;
}

const ConfigManager &MainWindow::configManager() const {
    return m_configManager;
}

} // namespace qworldclock

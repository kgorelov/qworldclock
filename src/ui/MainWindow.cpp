#include "ui/MainWindow.hpp"
#include "core/TimeEngine.hpp"
#include "ui/AnalogClockWidget.hpp"
#include "ui/ClockCardWidget.hpp"
#include "ui/ClockGridPanel.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
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
    : QMainWindow(parent) {
    setupUi();
}

void MainWindow::setupUi() {
    setWindowTitle(QStringLiteral("QWorldClock"));
    resize(900, 650);

    setupMenus();
    setupStatusBar();

    // Start central time engine
    TimeEngine::instance().start();

    // Initialize 2D Grid Model with user's local clock
    m_gridModel = new GridModel(this);
    m_gridModel->addClock({
        QStringLiteral("clock-local"),
        QTimeZone::systemTimeZone(),
        tr("Local Time"),
        0,
        0
    });

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
}

void MainWindow::onAddClockRequested(const QString &refId, Direction direction) {
    const QStringList presets = {
        QStringLiteral("London (Europe/London)"),
        QStringLiteral("New York (America/New_York)"),
        QStringLiteral("San Francisco (America/Los_Angeles)"),
        QStringLiteral("Tokyo (Asia/Tokyo)"),
        QStringLiteral("Geneva (Europe/Zurich)"),
        QStringLiteral("Sydney (Australia/Sydney)"),
        QStringLiteral("UTC (UTC)"),
        QStringLiteral("Custom...")
    };

    bool ok = false;
    const QString selected = QInputDialog::getItem(
        this,
        tr("Add Clock"),
        tr("Select Time Zone for new clock:"),
        presets,
        0,
        false,
        &ok);

    if (!ok || selected.isEmpty()) {
        return;
    }

    QByteArray tzId = "UTC";
    QString caption = QStringLiteral("World Clock");

    if (selected.contains(QStringLiteral("London"))) {
        tzId = "Europe/London";
        caption = QStringLiteral("London");
    } else if (selected.contains(QStringLiteral("New York"))) {
        tzId = "America/New_York";
        caption = QStringLiteral("New York");
    } else if (selected.contains(QStringLiteral("San Francisco"))) {
        tzId = "America/Los_Angeles";
        caption = QStringLiteral("San Francisco");
    } else if (selected.contains(QStringLiteral("Tokyo"))) {
        tzId = "Asia/Tokyo";
        caption = QStringLiteral("Tokyo");
    } else if (selected.contains(QStringLiteral("Geneva"))) {
        tzId = "Europe/Zurich";
        caption = QStringLiteral("Geneva");
    } else if (selected.contains(QStringLiteral("Sydney"))) {
        tzId = "Australia/Sydney";
        caption = QStringLiteral("Sydney");
    } else if (selected.contains(QStringLiteral("UTC"))) {
        tzId = "UTC";
        caption = QStringLiteral("UTC");
    } else {
        bool customOk = false;
        const QString customTz = QInputDialog::getText(
            this,
            tr("Custom Timezone"),
            tr("Enter IANA Timezone (e.g. Europe/Paris):"),
            QLineEdit::Normal,
            QStringLiteral("Europe/Paris"),
            &customOk);
        if (customOk && !customTz.isEmpty()) {
            tzId = customTz.toUtf8();
            caption = customTz.section('/', -1).replace('_', ' ');
        } else {
            return;
        }
    }

    QTimeZone tz(tzId);
    if (!tz.isValid()) {
        tz = QTimeZone::utc();
    }

    m_gridPanel->addClockRelative(refId, direction, tz, caption);
}

} // namespace qworldclock

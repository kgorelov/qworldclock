#include <QApplication>
#include <QDir>
#include <QMenuBar>
#include <QScrollArea>
#include <QScrollBar>
#include <QTest>
#include "ui/AnalogClockWidget.hpp"
#include "ui/CaptionLabel.hpp"
#include "ui/ClockCardWidget.hpp"
#include "ui/ClockGridPanel.hpp"
#include "ui/MainWindow.hpp"

using namespace qworldclock;

class TestWindow : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testStartupFit();
    void testMultipleClocksResponsive();
    void testAlignments();
    void testEndToEndPersistence();
    void testWorkingHoursAndOverride();
    void testCaptionFontSizeAndOverride();

private:
    QString m_testDir;
};

void TestWindow::initTestCase() {
    m_testDir = QDir::currentPath() + QStringLiteral("/build/test_scratch_window");
    QDir().mkpath(m_testDir);
    qApp->processEvents();
}

void TestWindow::cleanupTestCase() {
    QDir(m_testDir).removeRecursively();
}

void TestWindow::testStartupFit() {
    const QString cfgPath = m_testDir + QStringLiteral("/startup.cfg");
    MainWindow window(cfgPath);
    window.resize(900, 650);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    qApp->processEvents();

    QDir().mkpath(QStringLiteral("/home/kgorelov/.gemini/antigravity-cli/brain/88d5483d-5c0d-4588-9c75-e61e528b2895/scratch"));
    const QPixmap pix = window.grab();
    pix.save(QStringLiteral("/home/kgorelov/.gemini/antigravity-cli/brain/88d5483d-5c0d-4588-9c75-e61e528b2895/scratch/startup.png"));

    auto *scrollArea = window.findChild<QScrollArea *>();
    QVERIFY(scrollArea != nullptr);

    // On startup, the single clock should fit comfortably without vertical scrolling
    QVERIFY(!scrollArea->verticalScrollBar()->isVisible());

    auto *panel = window.findChild<ClockGridPanel *>();
    QVERIFY(panel != nullptr);

    auto *card = panel->cardWidget(QStringLiteral("clock-local"));
    QVERIFY(card != nullptr);

    // Card should be fully within the viewport bounds
    const QRect cardGeomInViewport = QRect(card->mapTo(scrollArea->viewport(), QPoint(0, 0)), card->size());
    const QRect viewportRect = scrollArea->viewport()->rect();

    QVERIFY2(viewportRect.contains(cardGeomInViewport),
             qPrintable(QStringLiteral("Card geom (%1,%2 %3x%4) should be within viewport (%5,%6 %7x%8)")
                            .arg(cardGeomInViewport.x())
                            .arg(cardGeomInViewport.y())
                            .arg(cardGeomInViewport.width())
                            .arg(cardGeomInViewport.height())
                            .arg(viewportRect.x())
                            .arg(viewportRect.y())
                            .arg(viewportRect.width())
                            .arg(viewportRect.height())));
}

void TestWindow::testMultipleClocksResponsive() {
    const QString cfgPath = m_testDir + QStringLiteral("/multi.cfg");
    MainWindow window(cfgPath);
    window.resize(900, 650);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    qApp->processEvents();

    auto *panel = window.findChild<ClockGridPanel *>();
    QVERIFY(panel != nullptr);

    // Add clock to right
    QVERIFY(panel->addClockRelative(QStringLiteral("clock-local"), Direction::Right,
                                    QTimeZone("America/New_York"), QStringLiteral("New York")));
    qApp->processEvents();

    QCOMPARE(panel->model()->count(), 2);
    auto *scrollArea = window.findChild<QScrollArea *>();
    QVERIFY(scrollArea != nullptr);

    // 2 clocks side-by-side in 900x650 should fit comfortably without scrollbars
    QVERIFY(!scrollArea->verticalScrollBar()->isVisible());
    QVERIFY(!scrollArea->horizontalScrollBar()->isVisible());
}

void TestWindow::testAlignments() {
    const QString cfgPath = m_testDir + QStringLiteral("/align.cfg");
    MainWindow window(cfgPath);
    window.resize(900, 650);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    qApp->processEvents();

    auto *panel = window.findChild<ClockGridPanel *>();
    QVERIFY(panel != nullptr);

    auto *card = panel->cardWidget(QStringLiteral("clock-local"));
    QVERIFY(card != nullptr);

    // Left alignment
    panel->setGridAlignment(Qt::AlignLeft);
    qApp->processEvents();
    const int leftX = card->mapTo(panel, QPoint(0, 0)).x();
    QVERIFY(leftX <= 30); // Near the 20px margin

    // Center alignment
    panel->setGridAlignment(Qt::AlignCenter);
    qApp->processEvents();
    const int centerX = card->mapTo(panel, QPoint(0, 0)).x();
    const int expectedCenterX = (panel->width() - card->width()) / 2;
    QVERIFY(qAbs(centerX - expectedCenterX) <= 10);

    // Right alignment
    panel->setGridAlignment(Qt::AlignRight);
    qApp->processEvents();
    const int rightX = card->mapTo(panel, QPoint(0, 0)).x();
    const int expectedRightX = panel->width() - card->width() - 20;
    QVERIFY(qAbs(rightX - expectedRightX) <= 10);
}

void TestWindow::testEndToEndPersistence() {
    const QString cfgPath = m_testDir + QStringLiteral("/e2e_persistence.cfg");

    // Session 1: Configure window, clocks, and settings, then close
    {
        MainWindow session1(cfgPath);
        session1.resize(1000, 700);
        session1.show();
        QVERIFY(QTest::qWaitForWindowExposed(&session1));
        qApp->processEvents();

        auto *panel = session1.findChild<ClockGridPanel *>();
        QVERIFY(panel != nullptr);

        // Set alignment to Right
        panel->setGridAlignment(Qt::AlignRight);

        // Set Fixed Sizing Mode with 250px clock size
        panel->setFixedClockSize(250);
        panel->setSizingMode(SizingMode::Fixed);

        // Add Tokyo clock Below
        QVERIFY(panel->addClockRelative(QStringLiteral("clock-local"), Direction::Below,
                                        QTimeZone("Asia/Tokyo"), QStringLiteral("Tokyo HQ")));

        // Hide menu bar
        session1.menuBar()->setVisible(false);

        session1.close();
    }

    // Verify config file was written
    QVERIFY(QFile::exists(cfgPath));

    // Session 2: Restore from saved config and verify exact state
    {
        MainWindow session2(cfgPath);
        session2.show();
        QVERIFY(QTest::qWaitForWindowExposed(&session2));
        qApp->processEvents();

        auto *panel = session2.findChild<ClockGridPanel *>();
        QVERIFY(panel != nullptr);

        // Clocks restored
        QCOMPARE(panel->model()->count(), 2);
        auto localCard = panel->cardWidget(QStringLiteral("clock-local"));
        QVERIFY(localCard != nullptr);
        QCOMPARE(localCard->gridRow(), 0);
        QCOMPARE(localCard->gridCol(), 0);

        // Alignment restored
        QCOMPARE(panel->gridAlignment(), Qt::AlignRight);

        // Sizing mode and size restored
        QCOMPARE(panel->sizingMode(), SizingMode::Fixed);
        QCOMPARE(panel->fixedClockSize(), 250);

        // Menu bar visibility restored
        QCOMPARE(session2.menuBar()->isVisible(), false);
    }
}

void TestWindow::testWorkingHoursAndOverride() {
    const QString cfgPath = m_testDir + QStringLiteral("/working_hours.cfg");

    // Session 1: Configure global working hours and clock override
    {
        MainWindow session1(cfgPath);
        session1.resize(900, 650);
        session1.show();
        QVERIFY(QTest::qWaitForWindowExposed(&session1));
        qApp->processEvents();

        auto *panel = session1.findChild<ClockGridPanel *>();
        QVERIFY(panel != nullptr);

        // Verify default global working hours
        QCOMPARE(panel->globalWorkingHours(), WorkingHours(8, 0, 18, 0));

        auto *localCard = panel->cardWidget(QStringLiteral("clock-local"));
        QVERIFY(localCard != nullptr);
        QVERIFY(!localCard->hasCustomWorkingHours());
        QCOMPARE(localCard->effectiveWorkingHours(), WorkingHours(8, 0, 18, 0));

        // Test day/night transition on clock widget
        auto *localAnalog = localCard->analogClock();
        QVERIFY(localAnalog != nullptr);
        QCOMPARE(localAnalog->workingHours(), WorkingHours(8, 0, 18, 0));

        // 12:00 UTC with UTC timezone => day
        localAnalog->setTimeZone(QTimeZone::utc());
        localAnalog->setTime(QDateTime(QDate(2026, 9, 19), QTime(12, 0, 0), QTimeZone::utc()));
        QVERIFY(!localAnalog->isNightTime());

        // 22:00 UTC with UTC timezone => night
        localAnalog->setTime(QDateTime(QDate(2026, 9, 19), QTime(22, 0, 0), QTimeZone::utc()));
        QVERIFY(localAnalog->isNightTime());

        // Change global working hours to 10:00 - 15:00
        panel->setGlobalWorkingHours(WorkingHours(10, 0, 15, 0));
        QCOMPARE(localCard->effectiveWorkingHours(), WorkingHours(10, 0, 15, 0));
        QCOMPARE(localAnalog->workingHours(), WorkingHours(10, 0, 15, 0));

        // At 09:00: outside 10:00 - 15:00 => night
        localAnalog->setTime(QDateTime(QDate(2026, 9, 19), QTime(9, 0, 0), QTimeZone::utc()));
        QVERIFY(localAnalog->isNightTime());

        // Add a second clock for Tokyo
        QVERIFY(panel->addClockRelative(QStringLiteral("clock-local"), Direction::Right,
                                        QTimeZone("Asia/Tokyo"), QStringLiteral("Tokyo HQ")));
        qApp->processEvents();

        const auto &clocks = panel->model()->clocks();
        QCOMPARE(clocks.size(), 2);
        QString tokyoId;
        for (const auto &c : clocks) {
            if (c.id != QStringLiteral("clock-local")) {
                tokyoId = c.id;
                break;
            }
        }
        QVERIFY(!tokyoId.isEmpty());

        auto *tokyoCard = panel->cardWidget(tokyoId);
        QVERIFY(tokyoCard != nullptr);
        // Initially inherits global working hours
        QVERIFY(!tokyoCard->hasCustomWorkingHours());
        QCOMPARE(tokyoCard->effectiveWorkingHours(), WorkingHours(10, 0, 15, 0));

        // Override Tokyo working hours to 06:00 - 22:00
        const WorkingHours tokyoHours(6, 0, 22, 0);
        QVERIFY(panel->model()->setClockWorkingHours(tokyoId, true, tokyoHours));
        tokyoCard->setHasCustomWorkingHours(true);
        tokyoCard->setCustomWorkingHours(tokyoHours);

        QVERIFY(tokyoCard->hasCustomWorkingHours());
        QCOMPARE(tokyoCard->effectiveWorkingHours(), tokyoHours);
        QCOMPARE(tokyoCard->analogClock()->workingHours(), tokyoHours);

        // Tokyo analog clock: local time 08:00 (UTC 23:00 previous day)
        // 08:00 is within 06:00 - 22:00 => day!
        tokyoCard->analogClock()->setTime(QDateTime(QDate(2026, 9, 19), QTime(23, 0, 0), QTimeZone::utc()));
        // Tokyo is UTC+9 => 23:00 + 9h = 08:00 next day
        QVERIFY(!tokyoCard->analogClock()->isNightTime());

        // At Tokyo local time 02:00 (UTC 17:00) => outside 06:00 - 22:00 => night
        tokyoCard->analogClock()->setTime(QDateTime(QDate(2026, 9, 19), QTime(17, 0, 0), QTimeZone::utc()));
        QVERIFY(tokyoCard->analogClock()->isNightTime());

        session1.close();
    }

    // Verify persistence in Session 2
    {
        MainWindow session2(cfgPath);
        session2.show();
        QVERIFY(QTest::qWaitForWindowExposed(&session2));
        qApp->processEvents();

        auto *panel = session2.findChild<ClockGridPanel *>();
        QVERIFY(panel != nullptr);

        // Global working hours restored
        QCOMPARE(panel->globalWorkingHours(), WorkingHours(10, 0, 15, 0));

        auto *localCard = panel->cardWidget(QStringLiteral("clock-local"));
        QVERIFY(localCard != nullptr);
        QVERIFY(!localCard->hasCustomWorkingHours());
        QCOMPARE(localCard->effectiveWorkingHours(), WorkingHours(10, 0, 15, 0));

        // Tokyo card restored with custom working hours
        const auto &clocks = panel->model()->clocks();
        QCOMPARE(clocks.size(), 2);
        QString tokyoId;
        for (const auto &c : clocks) {
            if (c.id != QStringLiteral("clock-local")) {
                tokyoId = c.id;
                break;
            }
        }
        QVERIFY(!tokyoId.isEmpty());

        auto *tokyoCard = panel->cardWidget(tokyoId);
        QVERIFY(tokyoCard != nullptr);
        QVERIFY(tokyoCard->hasCustomWorkingHours());
        QCOMPARE(tokyoCard->customWorkingHours(), WorkingHours(6, 0, 22, 0));
        QCOMPARE(tokyoCard->effectiveWorkingHours(), WorkingHours(6, 0, 22, 0));
        QCOMPARE(tokyoCard->analogClock()->workingHours(), WorkingHours(6, 0, 22, 0));

        // Reset Tokyo card back to global
        QVERIFY(panel->model()->setClockWorkingHours(tokyoId, false));
        tokyoCard->setHasCustomWorkingHours(false);
        QVERIFY(!tokyoCard->hasCustomWorkingHours());
        QCOMPARE(tokyoCard->effectiveWorkingHours(), WorkingHours(10, 0, 15, 0));
        QCOMPARE(tokyoCard->analogClock()->workingHours(), WorkingHours(10, 0, 15, 0));
    }
}

void TestWindow::testCaptionFontSizeAndOverride() {
    const QString cfgPath = m_testDir + QStringLiteral("/caption_font_size.cfg");

    // Session 1: Configure global caption font size and per-clock override
    {
        MainWindow session1(cfgPath);
        session1.show();
        QVERIFY(QTest::qWaitForWindowExposed(&session1));
        qApp->processEvents();

        auto *panel = session1.findChild<ClockGridPanel *>();
        QVERIFY(panel != nullptr);

        // Default global caption font size is 0 (auto)
        QCOMPARE(panel->globalCaptionFontSize(), 0);

        auto *localCard = panel->cardWidget(QStringLiteral("clock-local"));
        QVERIFY(localCard != nullptr);
        QCOMPARE(localCard->effectiveCaptionFontSize(), 0);
        QCOMPARE(localCard->captionLabel()->fontSize(), 0);

        // Change global caption font size to 17
        panel->setGlobalCaptionFontSize(17);
        QCOMPARE(panel->globalCaptionFontSize(), 17);
        QCOMPARE(localCard->effectiveCaptionFontSize(), 17);
        QCOMPARE(localCard->captionLabel()->fontSize(), 17);

        // Add a second clock (Tokyo)
        QVERIFY(panel->addClockRelative(QStringLiteral("clock-local"),
                                        Direction::Right,
                                        QTimeZone("Asia/Tokyo"),
                                        QStringLiteral("Tokyo HQ")));
        qApp->processEvents();

        const auto &clocks = panel->model()->clocks();
        QCOMPARE(clocks.size(), 2);
        QString tokyoId;
        for (const auto &c : clocks) {
            if (c.id != QStringLiteral("clock-local")) {
                tokyoId = c.id;
                break;
            }
        }
        QVERIFY(!tokyoId.isEmpty());

        auto *tokyoCard = panel->cardWidget(tokyoId);
        QVERIFY(tokyoCard != nullptr);
        // Initially inherits global caption font size
        QVERIFY(!tokyoCard->hasCustomCaptionFontSize());
        QCOMPARE(tokyoCard->effectiveCaptionFontSize(), 17);
        QCOMPARE(tokyoCard->captionLabel()->fontSize(), 17);

        // Override Tokyo caption font size to 11
        QVERIFY(panel->model()->setClockCaptionFontSize(tokyoId, true, 11));
        tokyoCard->setHasCustomCaptionFontSize(true);
        tokyoCard->setCaptionFontSize(11);

        QVERIFY(tokyoCard->hasCustomCaptionFontSize());
        QCOMPARE(tokyoCard->effectiveCaptionFontSize(), 11);
        QCOMPARE(tokyoCard->captionLabel()->fontSize(), 11);

        // Changing global to 20 should affect localCard but not tokyoCard
        panel->setGlobalCaptionFontSize(20);
        QCOMPARE(localCard->effectiveCaptionFontSize(), 20);
        QCOMPARE(localCard->captionLabel()->fontSize(), 20);
        QCOMPARE(tokyoCard->effectiveCaptionFontSize(), 11);
        QCOMPARE(tokyoCard->captionLabel()->fontSize(), 11);

        session1.close();
    }

    // Verify persistence in Session 2
    {
        MainWindow session2(cfgPath);
        session2.show();
        QVERIFY(QTest::qWaitForWindowExposed(&session2));
        qApp->processEvents();

        auto *panel = session2.findChild<ClockGridPanel *>();
        QVERIFY(panel != nullptr);

        // Global caption font size restored to 20
        QCOMPARE(panel->globalCaptionFontSize(), 20);

        auto *localCard = panel->cardWidget(QStringLiteral("clock-local"));
        QVERIFY(localCard != nullptr);
        QVERIFY(!localCard->hasCustomCaptionFontSize());
        QCOMPARE(localCard->effectiveCaptionFontSize(), 20);
        QCOMPARE(localCard->captionLabel()->fontSize(), 20);

        // Tokyo card restored with custom caption font size 11
        const auto &clocks = panel->model()->clocks();
        QCOMPARE(clocks.size(), 2);
        QString tokyoId;
        for (const auto &c : clocks) {
            if (c.id != QStringLiteral("clock-local")) {
                tokyoId = c.id;
                break;
            }
        }
        QVERIFY(!tokyoId.isEmpty());

        auto *tokyoCard = panel->cardWidget(tokyoId);
        QVERIFY(tokyoCard != nullptr);
        QVERIFY(tokyoCard->hasCustomCaptionFontSize());
        QCOMPARE(tokyoCard->captionFontSize(), 11);
        QCOMPARE(tokyoCard->effectiveCaptionFontSize(), 11);
        QCOMPARE(tokyoCard->captionLabel()->fontSize(), 11);

        // Reset Tokyo card back to global
        QVERIFY(panel->model()->setClockCaptionFontSize(tokyoId, false));
        tokyoCard->setHasCustomCaptionFontSize(false);
        QVERIFY(!tokyoCard->hasCustomCaptionFontSize());
        QCOMPARE(tokyoCard->effectiveCaptionFontSize(), 20);
        QCOMPARE(tokyoCard->captionLabel()->fontSize(), 20);
    }
}

QTEST_MAIN(TestWindow)
#include "TestWindow.moc"

#include <QApplication>
#include <QDir>
#include <QMenuBar>
#include <QScrollArea>
#include <QScrollBar>
#include <QTest>
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

QTEST_MAIN(TestWindow)
#include "TestWindow.moc"

#include <QApplication>
#include <QDir>
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
    void testStartupFit();
    void testMultipleClocksResponsive();
    void testAlignments();
};

void TestWindow::initTestCase() {
    qApp->processEvents();
}

void TestWindow::testStartupFit() {
    MainWindow window;
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
    MainWindow window;
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
    MainWindow window;
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

QTEST_MAIN(TestWindow)
#include "TestWindow.moc"

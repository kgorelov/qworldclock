#include <QTest>
#include "core/GridModel.hpp"

using namespace qworldclock;

class TestGridModel : public QObject {
    Q_OBJECT

private slots:
    void testInitialState();
    void testAddClock();
    void testInsertRight();
    void testInsertLeft();
    void testInsertBelow();
    void testInsertAbove();
    void testSingleClockGuard();
    void testRemoveAndNormalize();
    void testSwapAndMove();
    void testClockWorkingHours();
    void testClockCaptionFontSize();
    void testRestartOverlapBug();
    void testRestartAfterInsertLeft();
    void testCorruptedConfigRecovery();
};

void TestGridModel::testInitialState() {
    GridModel model;
    QCOMPARE(model.count(), 0);
    QCOMPARE(model.canRemove(), false);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 0);
}

void TestGridModel::testAddClock() {
    GridModel model;
    ClockItem item1{QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0};
    QVERIFY(model.addClock(item1));

    QCOMPARE(model.count(), 1);
    QCOMPARE(model.canRemove(), false); // Single clock guard
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.columnCount(), 1);

    // Duplicate ID should fail
    QVERIFY(!model.addClock(item1));
}

void TestGridModel::testInsertRight() {
    GridModel model;
    model.addClock({QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});

    ClockItem item2{QStringLiteral("c2"), QTimeZone::systemTimeZone(), QStringLiteral("London"), 0, 0};
    QVERIFY(model.insertRelative(QStringLiteral("c1"), Direction::Right, item2));

    QCOMPARE(model.count(), 2);
    auto c1 = model.clockById(QStringLiteral("c1"));
    auto c2 = model.clockById(QStringLiteral("c2"));
    QVERIFY(c1.has_value());
    QVERIFY(c2.has_value());
    QCOMPARE(c1->row, 0);
    QCOMPARE(c1->col, 0);
    QCOMPARE(c2->row, 0);
    QCOMPARE(c2->col, 1);
}

void TestGridModel::testInsertLeft() {
    GridModel model;
    model.addClock({QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});

    ClockItem item2{QStringLiteral("c2"), QTimeZone::systemTimeZone(), QStringLiteral("NY"), 0, 0};
    QVERIFY(model.insertRelative(QStringLiteral("c1"), Direction::Left, item2));

    QCOMPARE(model.count(), 2);
    auto c1 = model.clockById(QStringLiteral("c1"));
    auto c2 = model.clockById(QStringLiteral("c2"));
    QVERIFY(c1.has_value());
    QVERIFY(c2.has_value());
    QCOMPARE(c2->row, 0);
    QCOMPARE(c2->col, 0); // c2 was inserted at col 0
    QCOMPARE(c1->row, 0);
    QCOMPARE(c1->col, 1); // c1 shifted right to col 1
}

void TestGridModel::testInsertBelow() {
    GridModel model;
    model.addClock({QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});

    ClockItem item2{QStringLiteral("c2"), QTimeZone::systemTimeZone(), QStringLiteral("Tokyo"), 0, 0};
    QVERIFY(model.insertRelative(QStringLiteral("c1"), Direction::Below, item2));

    QCOMPARE(model.count(), 2);
    auto c1 = model.clockById(QStringLiteral("c1"));
    auto c2 = model.clockById(QStringLiteral("c2"));
    QCOMPARE(c1->row, 0);
    QCOMPARE(c1->col, 0);
    QCOMPARE(c2->row, 1);
    QCOMPARE(c2->col, 0);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.columnCount(), 1);
}

void TestGridModel::testInsertAbove() {
    GridModel model;
    model.addClock({QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});

    ClockItem item2{QStringLiteral("c2"), QTimeZone::systemTimeZone(), QStringLiteral("Paris"), 0, 0};
    QVERIFY(model.insertRelative(QStringLiteral("c1"), Direction::Above, item2));

    QCOMPARE(model.count(), 2);
    auto c1 = model.clockById(QStringLiteral("c1"));
    auto c2 = model.clockById(QStringLiteral("c2"));
    QCOMPARE(c2->row, 0); // c2 inserted at top
    QCOMPARE(c2->col, 0);
    QCOMPARE(c1->row, 1); // c1 shifted down
    QCOMPARE(c1->col, 0);
}

void TestGridModel::testSingleClockGuard() {
    GridModel model;
    model.addClock({QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});

    // Cannot remove when only 1 clock remains
    QVERIFY(!model.canRemove());
    QVERIFY(!model.removeClock(QStringLiteral("c1")));
    QCOMPARE(model.count(), 1);

    // Add a second clock
    model.insertRelative(QStringLiteral("c1"), Direction::Right,
                         {QStringLiteral("c2"), QTimeZone::systemTimeZone(), QStringLiteral("London"), 0, 0});
    QVERIFY(model.canRemove());

    // Now removal is allowed
    QVERIFY(model.removeClock(QStringLiteral("c2")));
    QCOMPARE(model.count(), 1);

    // Again, cannot remove last clock
    QVERIFY(!model.canRemove());
    QVERIFY(!model.removeClock(QStringLiteral("c1")));
}

void TestGridModel::testRemoveAndNormalize() {
    GridModel model;
    model.addClock({QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});
    model.insertRelative(QStringLiteral("c1"), Direction::Right,
                         {QStringLiteral("c2"), QTimeZone::systemTimeZone(), QStringLiteral("London"), 0, 0});
    model.insertRelative(QStringLiteral("c2"), Direction::Right,
                         {QStringLiteral("c3"), QTimeZone::systemTimeZone(), QStringLiteral("Tokyo"), 0, 0});

    QCOMPARE(model.columnCount(), 3);

    // Remove middle clock
    QVERIFY(model.removeClock(QStringLiteral("c2")));
    QCOMPARE(model.count(), 2);

    auto c1 = model.clockById(QStringLiteral("c1"));
    auto c3 = model.clockById(QStringLiteral("c3"));
    QCOMPARE(c1->col, 0);
    QCOMPARE(c3->col, 1); // Normalized from 2 to 1
    QCOMPARE(model.columnCount(), 2);
}

void TestGridModel::testSwapAndMove() {
    GridModel model;
    model.addClock({QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});
    model.insertRelative(QStringLiteral("c1"), Direction::Right,
                         {QStringLiteral("c2"), QTimeZone::systemTimeZone(), QStringLiteral("London"), 0, 0});

    QVERIFY(model.swapClocks(QStringLiteral("c1"), QStringLiteral("c2")));
    auto c1 = model.clockById(QStringLiteral("c1"));
    auto c2 = model.clockById(QStringLiteral("c2"));
    QCOMPARE(c1->col, 1);
    QCOMPARE(c2->col, 0);

    QVERIFY(model.moveClock(QStringLiteral("c1"), 1, 0));
    c1 = model.clockById(QStringLiteral("c1"));
    QCOMPARE(c1->row, 1);
    QCOMPARE(c1->col, 0);
}

void TestGridModel::testClockWorkingHours() {
    GridModel model;
    model.addClock({QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});

    auto c1 = model.clockById(QStringLiteral("c1"));
    QVERIFY(c1.has_value());
    QCOMPARE(c1->hasCustomWorkingHours, false);

    bool layoutChangedEmitted = false;
    connect(&model, &GridModel::layoutChanged, [&layoutChangedEmitted]() {
        layoutChangedEmitted = true;
    });

    const WorkingHours custom(7, 30, 16, 0);
    QVERIFY(model.setClockWorkingHours(QStringLiteral("c1"), true, custom));
    QVERIFY(layoutChangedEmitted);

    c1 = model.clockById(QStringLiteral("c1"));
    QVERIFY(c1.has_value());
    QCOMPARE(c1->hasCustomWorkingHours, true);
    QCOMPARE(c1->customWorkingHours, custom);

    // Reset back to global
    layoutChangedEmitted = false;
    QVERIFY(model.setClockWorkingHours(QStringLiteral("c1"), false));
    QVERIFY(layoutChangedEmitted);

    c1 = model.clockById(QStringLiteral("c1"));
    QVERIFY(c1.has_value());
    QCOMPARE(c1->hasCustomWorkingHours, false);

    // Non-existent ID returns false
    QVERIFY(!model.setClockWorkingHours(QStringLiteral("non-existent"), true, custom));
}

void TestGridModel::testClockCaptionFontSize() {
    GridModel model;
    model.addClock({QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});

    auto c1 = model.clockById(QStringLiteral("c1"));
    QVERIFY(c1.has_value());
    QCOMPARE(c1->hasCustomCaptionFontSize, false);
    QCOMPARE(c1->customCaptionFontSize, 0);

    bool layoutChangedEmitted = false;
    connect(&model, &GridModel::layoutChanged, [&layoutChangedEmitted]() {
        layoutChangedEmitted = true;
    });

    QVERIFY(model.setClockCaptionFontSize(QStringLiteral("c1"), true, 18));
    QVERIFY(layoutChangedEmitted);

    c1 = model.clockById(QStringLiteral("c1"));
    QVERIFY(c1.has_value());
    QCOMPARE(c1->hasCustomCaptionFontSize, true);
    QCOMPARE(c1->customCaptionFontSize, 18);

    // Reset back to global
    layoutChangedEmitted = false;
    QVERIFY(model.setClockCaptionFontSize(QStringLiteral("c1"), false));
    QVERIFY(layoutChangedEmitted);

    c1 = model.clockById(QStringLiteral("c1"));
    QVERIFY(c1.has_value());
    QCOMPARE(c1->hasCustomCaptionFontSize, false);

    // Non-existent ID returns false
    QVERIFY(!model.setClockCaptionFontSize(QStringLiteral("non-existent"), true, 18));
}

void TestGridModel::testRestartOverlapBug() {
    GridModel model;
    model.addClock({QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});
    model.insertRelative(QStringLiteral("c1"), Direction::Right,
                         {QStringLiteral("c2"), QTimeZone::systemTimeZone(), QStringLiteral("London"), 0, 0});

    // Swap clocks so c1 is at col 1 and c2 is at col 0
    QVERIFY(model.swapClocks(QStringLiteral("c1"), QStringLiteral("c2")));
    QCOMPARE(model.clockById(QStringLiteral("c1"))->col, 1);
    QCOMPARE(model.clockById(QStringLiteral("c2"))->col, 0);

    // Simulate saving to config and reloading into a new GridModel via addClock
    GridModel reloaded;
    for (const auto &item : model.clocks()) {
        reloaded.addClock(item);
    }

    // Now check if positions were preserved
    auto rc1 = reloaded.clockById(QStringLiteral("c1"));
    auto rc2 = reloaded.clockById(QStringLiteral("c2"));
    QVERIFY(rc1.has_value());
    QVERIFY(rc2.has_value());

    // Expect: c1 at col 1, c2 at col 0, and columnCount == 2
    QCOMPARE(rc1->col, 1);
    QCOMPARE(rc2->col, 0);
    QCOMPARE(reloaded.columnCount(), 2);
}

void TestGridModel::testRestartAfterInsertLeft() {
    GridModel model;
    model.addClock({QStringLiteral("local"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0});
    // Add new clock to the left of local clock
    QVERIFY(model.insertRelative(QStringLiteral("local"), Direction::Left,
                                 {QStringLiteral("ny"), QTimeZone::systemTimeZone(), QStringLiteral("New York"), 0, 0}));

    // In model: ny at (0, 0), local at (0, 1)
    auto ny = model.clockById(QStringLiteral("ny"));
    auto local = model.clockById(QStringLiteral("local"));
    QVERIFY(ny.has_value());
    QVERIFY(local.has_value());
    QCOMPARE(ny->col, 0);
    QCOMPARE(local->col, 1);

    // Simulate saving and restoring via setClocks
    GridModel reloaded;
    reloaded.setClocks(model.clocks());

    auto rny = reloaded.clockById(QStringLiteral("ny"));
    auto rlocal = reloaded.clockById(QStringLiteral("local"));
    QVERIFY(rny.has_value());
    QVERIFY(rlocal.has_value());
    QCOMPARE(rny->col, 0);
    QCOMPARE(rlocal->col, 1);
    QCOMPARE(reloaded.columnCount(), 2);
    QVERIFY(rny->col != rlocal->col);
}

void TestGridModel::testCorruptedConfigRecovery() {
    // Simulate loading a config that already contains corrupted duplicate positions
    std::vector<ClockItem> corrupted = {
        {QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Clock 1"), 0, 0},
        {QStringLiteral("c2"), QTimeZone::systemTimeZone(), QStringLiteral("Clock 2"), 0, 0},
        {QStringLiteral("c3"), QTimeZone::systemTimeZone(), QStringLiteral("Clock 3"), 0, 0}
    };

    GridModel model;
    model.setClocks(corrupted);

    QCOMPARE(model.count(), 3);
    auto c1 = model.clockById(QStringLiteral("c1"));
    auto c2 = model.clockById(QStringLiteral("c2"));
    auto c3 = model.clockById(QStringLiteral("c3"));
    QVERIFY(c1.has_value());
    QVERIFY(c2.has_value());
    QVERIFY(c3.has_value());

    // All three clocks must have unique columns
    QVERIFY(c1->col != c2->col);
    QVERIFY(c1->col != c3->col);
    QVERIFY(c2->col != c3->col);
    QCOMPARE(model.columnCount(), 3);
}

QTEST_MAIN(TestGridModel)
#include "TestGridModel.moc"

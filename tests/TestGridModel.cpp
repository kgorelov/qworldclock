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

QTEST_MAIN(TestGridModel)
#include "TestGridModel.moc"

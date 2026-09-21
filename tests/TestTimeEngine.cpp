#include <QSignalSpy>
#include <QTest>
#include <QTimeZone>
#include "core/TimeEngine.hpp"

using namespace qworldclock;

class TestTimeEngine : public QObject {
    Q_OBJECT

private slots:
    void testSingleton();
    void testStartStop();
    void testSignalEmission();
    void testTimeZoneOffset();
    void testDayRollover();
    void testLondonWinterTimeShift();
};

void TestTimeEngine::testSingleton() {
    TimeEngine &engine1 = TimeEngine::instance();
    TimeEngine &engine2 = TimeEngine::instance();
    QCOMPARE(&engine1, &engine2);

    const QDateTime utc = engine1.currentUtcTime();
    QVERIFY(utc.isValid());
    QCOMPARE(utc.timeSpec(), Qt::UTC);
}

void TestTimeEngine::testStartStop() {
    TimeEngine &engine = TimeEngine::instance();

    engine.stop();
    QVERIFY(!engine.isRunning());

    engine.start();
    QVERIFY(engine.isRunning());

    // Multiple calls to start should keep it running
    engine.start();
    QVERIFY(engine.isRunning());

    engine.stop();
    QVERIFY(!engine.isRunning());
}

void TestTimeEngine::testSignalEmission() {
    TimeEngine &engine = TimeEngine::instance();
    engine.stop();

    QSignalSpy spy(&engine, &TimeEngine::tick);
    QVERIFY(spy.isValid());

    // Starting the engine triggers an immediate initial tick
    engine.start();
    QVERIFY(spy.count() >= 1);

    const QList<QVariant> arguments = spy.takeFirst();
    const QDateTime emittedUtc = arguments.at(0).toDateTime();
    QVERIFY(emittedUtc.isValid());
    QCOMPARE(emittedUtc.timeSpec(), Qt::UTC);

    engine.stop();
}

void TestTimeEngine::testTimeZoneOffset() {
    const QDateTime utcTime(QDate(2026, 6, 15), QTime(12, 0, 0), QTimeZone::UTC);

    const QTimeZone tzLondon("Europe/London");
    if (tzLondon.isValid()) {
        // In June, London is in BST (UTC+1, 3600s)
        QCOMPARE(tzLondon.offsetFromUtc(utcTime), 3600);
    }

    const QTimeZone tzTokyo("Asia/Tokyo");
    if (tzTokyo.isValid()) {
        // Tokyo is UTC+9 year-round (32400s)
        QCOMPARE(tzTokyo.offsetFromUtc(utcTime), 32400);
    }

    const QTimeZone tzKathmandu("Asia/Kathmandu");
    if (tzKathmandu.isValid()) {
        // Nepal is UTC+5:45 (20700s)
        QCOMPARE(tzKathmandu.offsetFromUtc(utcTime), 20700);
    }

    const QTimeZone tzHonolulu("Pacific/Honolulu");
    if (tzHonolulu.isValid()) {
        // Hawaii is UTC-10 year-round (-36000s)
        QCOMPARE(tzHonolulu.offsetFromUtc(utcTime), -36000);
    }
}

void TestTimeEngine::testDayRollover() {
    // 2026-09-20 23:30:00 UTC
    const QDateTime lateUtc(QDate(2026, 9, 20), QTime(23, 30, 0), QTimeZone::UTC);

    const QTimeZone tzTokyo("Asia/Tokyo");
    if (tzTokyo.isValid()) {
        const QDateTime tokyoTime = lateUtc.toTimeZone(tzTokyo);
        // Tokyo is UTC+9 -> 2026-09-21 08:30:00
        QCOMPARE(tokyoTime.date(), QDate(2026, 9, 21));
        QCOMPARE(lateUtc.date().daysTo(tokyoTime.date()), 1);
    }

    // 2026-09-20 02:00:00 UTC
    const QDateTime earlyUtc(QDate(2026, 9, 20), QTime(2, 0, 0), QTimeZone::UTC);
    const QTimeZone tzHonolulu("Pacific/Honolulu");
    if (tzHonolulu.isValid()) {
        const QDateTime honoluluTime = earlyUtc.toTimeZone(tzHonolulu);
        // Honolulu is UTC-10 -> 2026-09-19 16:00:00
        QCOMPARE(honoluluTime.date(), QDate(2026, 9, 19));
        QCOMPARE(earlyUtc.date().daysTo(honoluluTime.date()), -1);
    }
}

void TestTimeEngine::testLondonWinterTimeShift() {
    const QTimeZone tzLondon("Europe/London");
    QVERIFY(tzLondon.isValid());

    // In 2026, the UK transitions from BST (UTC+1) to GMT (UTC+0) on Sunday, 25 October at 01:00 UTC (02:00 BST).
    // 1. One second before transition: 2026-10-25 00:59:59 UTC
    const QDateTime beforeShiftUtc(QDate(2026, 10, 25), QTime(0, 59, 59), QTimeZone::UTC);
    QCOMPARE(tzLondon.offsetFromUtc(beforeShiftUtc), 3600);
    QVERIFY(tzLondon.isDaylightTime(beforeShiftUtc));

    const QDateTime londonBefore = beforeShiftUtc.toTimeZone(tzLondon);
    QCOMPARE(londonBefore.time(), QTime(1, 59, 59));

    // 2. Exactly at transition: 2026-10-25 01:00:00 UTC -> Clocks fall back to 01:00:00 GMT (UTC+0)
    const QDateTime atShiftUtc(QDate(2026, 10, 25), QTime(1, 0, 0), QTimeZone::UTC);
    QCOMPARE(tzLondon.offsetFromUtc(atShiftUtc), 0);
    QVERIFY(!tzLondon.isDaylightTime(atShiftUtc));

    const QDateTime londonAfter = atShiftUtc.toTimeZone(tzLondon);
    QCOMPARE(londonAfter.time(), QTime(1, 0, 0));

    // 3. Middle of winter: 2026-12-15 12:00:00 UTC -> UTC+0
    const QDateTime winterUtc(QDate(2026, 12, 15), QTime(12, 0, 0), QTimeZone::UTC);
    QCOMPARE(tzLondon.offsetFromUtc(winterUtc), 0);
    QVERIFY(!tzLondon.isDaylightTime(winterUtc));

    const QDateTime londonWinter = winterUtc.toTimeZone(tzLondon);
    QCOMPARE(londonWinter.time(), QTime(12, 0, 0));
}

QTEST_MAIN(TestTimeEngine)
#include "TestTimeEngine.moc"

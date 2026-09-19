#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTest>
#include "core/ConfigManager.hpp"

using namespace qworldclock;

class TestConfigManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testDefaultConfig();
    void testSaveAndLoad();
    void testMissingFileFallback();
    void testCorruptFileFallback();
    void testDirectoryCreation();
    void testStringConversions();
    void testWorkingHoursLogic();

private:
    QString m_testDir;
};

void TestConfigManager::initTestCase() {
    m_testDir = QDir::currentPath() + QStringLiteral("/build/test_scratch_config");
    QDir().mkpath(m_testDir);
}

void TestConfigManager::cleanupTestCase() {
    QDir(m_testDir).removeRecursively();
}

void TestConfigManager::testDefaultConfig() {
    const AppConfig def = ConfigManager::defaultConfig();
    QCOMPARE(def.version, QStringLiteral("1.0.0"));
    QCOMPARE(def.sizingMode, QStringLiteral("responsive"));
    QCOMPARE(def.fixedClockSize, 190);
    QCOMPARE(def.alignment, QStringLiteral("center"));
    QCOMPARE(def.showMenuBar, true);
    QCOMPARE(def.showStatusBar, true);
    QCOMPARE(def.showSeconds, true);
    QCOMPARE(def.showDayNight, true);
    QCOMPARE(def.workingHours, WorkingHours(8, 0, 18, 0));
    QCOMPARE(def.window.width, 900);
    QCOMPARE(def.window.height, 650);
    QCOMPARE(def.clocks.size(), 1);
    QCOMPARE(def.clocks[0].id, QStringLiteral("clock-local"));
    QCOMPARE(def.clocks[0].caption, QStringLiteral("Local Time"));
    QCOMPARE(def.clocks[0].row, 0);
    QCOMPARE(def.clocks[0].col, 0);
    QCOMPARE(def.clocks[0].hasCustomWorkingHours, false);
}

void TestConfigManager::testSaveAndLoad() {
    const QString filePath = m_testDir + QStringLiteral("/custom_config.cfg");
    ConfigManager manager(filePath);

    AppConfig savedCfg;
    savedCfg.version = QStringLiteral("1.0.0");
    savedCfg.sizingMode = QStringLiteral("fixed");
    savedCfg.fixedClockSize = 250;
    savedCfg.alignment = QStringLiteral("right");
    savedCfg.showMenuBar = false;
    savedCfg.showStatusBar = false;
    savedCfg.showSeconds = false;
    savedCfg.showDayNight = false;
    savedCfg.workingHours = WorkingHours(9, 15, 17, 45);

    savedCfg.window.width = 1200;
    savedCfg.window.height = 800;
    savedCfg.window.x = 100;
    savedCfg.window.y = 150;
    savedCfg.window.maximized = true;

    ClockItem c1{QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0, false, WorkingHours(8, 0, 18, 0)};
    ClockItem c2{QStringLiteral("c2"), QTimeZone("Europe/London"), QStringLiteral("London Office"), 0, 1, true, WorkingHours(7, 30, 16, 0)};
    ClockItem c3{QStringLiteral("c3"), QTimeZone("Asia/Tokyo"), QStringLiteral("Tokyo HQ"), 1, 0, false, WorkingHours(8, 0, 18, 0)};

    savedCfg.clocks = {c1, c2, c3};

    QVERIFY(manager.save(savedCfg));
    QVERIFY(QFile::exists(filePath));

    const AppConfig loadedCfg = manager.load();
    QCOMPARE(loadedCfg.version, savedCfg.version);
    QCOMPARE(loadedCfg.sizingMode, savedCfg.sizingMode);
    QCOMPARE(loadedCfg.fixedClockSize, savedCfg.fixedClockSize);
    QCOMPARE(loadedCfg.alignment, savedCfg.alignment);
    QCOMPARE(loadedCfg.showMenuBar, savedCfg.showMenuBar);
    QCOMPARE(loadedCfg.showStatusBar, savedCfg.showStatusBar);
    QCOMPARE(loadedCfg.showSeconds, savedCfg.showSeconds);
    QCOMPARE(loadedCfg.showDayNight, savedCfg.showDayNight);
    QCOMPARE(loadedCfg.workingHours, savedCfg.workingHours);

    QCOMPARE(loadedCfg.window.width, savedCfg.window.width);
    QCOMPARE(loadedCfg.window.height, savedCfg.window.height);
    QCOMPARE(loadedCfg.window.x, savedCfg.window.x);
    QCOMPARE(loadedCfg.window.y, savedCfg.window.y);
    QCOMPARE(loadedCfg.window.maximized, savedCfg.window.maximized);

    QCOMPARE(loadedCfg.clocks.size(), 3);
    QCOMPARE(loadedCfg.clocks[0].id, QStringLiteral("c1"));
    QCOMPARE(loadedCfg.clocks[0].caption, QStringLiteral("Local"));
    QCOMPARE(loadedCfg.clocks[0].row, 0);
    QCOMPARE(loadedCfg.clocks[0].col, 0);
    QCOMPARE(loadedCfg.clocks[0].hasCustomWorkingHours, false);

    QCOMPARE(loadedCfg.clocks[1].id, QStringLiteral("c2"));
    QCOMPARE(loadedCfg.clocks[1].caption, QStringLiteral("London Office"));
    QCOMPARE(loadedCfg.clocks[1].timeZone.id(), QByteArray("Europe/London"));
    QCOMPARE(loadedCfg.clocks[1].row, 0);
    QCOMPARE(loadedCfg.clocks[1].col, 1);
    QCOMPARE(loadedCfg.clocks[1].hasCustomWorkingHours, true);
    QCOMPARE(loadedCfg.clocks[1].customWorkingHours, WorkingHours(7, 30, 16, 0));

    QCOMPARE(loadedCfg.clocks[2].id, QStringLiteral("c3"));
    QCOMPARE(loadedCfg.clocks[2].caption, QStringLiteral("Tokyo HQ"));
    QCOMPARE(loadedCfg.clocks[2].timeZone.id(), QByteArray("Asia/Tokyo"));
    QCOMPARE(loadedCfg.clocks[2].row, 1);
    QCOMPARE(loadedCfg.clocks[2].col, 0);
    QCOMPARE(loadedCfg.clocks[2].hasCustomWorkingHours, false);
}

void TestConfigManager::testMissingFileFallback() {
    ConfigManager manager(m_testDir + QStringLiteral("/non_existent.cfg"));
    const AppConfig cfg = manager.load();
    QCOMPARE(cfg.version, QStringLiteral("1.0.0"));
    QCOMPARE(cfg.clocks.size(), 1);
    QCOMPARE(cfg.clocks[0].id, QStringLiteral("clock-local"));
}

void TestConfigManager::testCorruptFileFallback() {
    const QString corruptPath = m_testDir + QStringLiteral("/corrupt.cfg");
    QFile file(corruptPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("this is completely invalid [[ TOML content {{{");
    file.close();

    ConfigManager manager(corruptPath);
    const AppConfig cfg = manager.load();
    QCOMPARE(cfg.version, QStringLiteral("1.0.0"));
    QCOMPARE(cfg.clocks.size(), 1);
    QCOMPARE(cfg.clocks[0].id, QStringLiteral("clock-local"));
}

void TestConfigManager::testDirectoryCreation() {
    const QString nestedPath = m_testDir + QStringLiteral("/deep/sub/dir/test.cfg");
    ConfigManager manager(nestedPath);
    QVERIFY(manager.save(ConfigManager::defaultConfig()));
    QVERIFY(QFile::exists(nestedPath));
}

void TestConfigManager::testStringConversions() {
    QCOMPARE(ConfigManager::stringToAlignment(QStringLiteral("left")), Qt::AlignLeft);
    QCOMPARE(ConfigManager::stringToAlignment(QStringLiteral("LEFT")), Qt::AlignLeft);
    QCOMPARE(ConfigManager::stringToAlignment(QStringLiteral("right")), Qt::AlignRight);
    QCOMPARE(ConfigManager::stringToAlignment(QStringLiteral("center")), Qt::AlignCenter);
    QCOMPARE(ConfigManager::stringToAlignment(QStringLiteral("other")), Qt::AlignCenter);

    QCOMPARE(ConfigManager::alignmentToString(Qt::AlignLeft), QStringLiteral("left"));
    QCOMPARE(ConfigManager::alignmentToString(Qt::AlignRight), QStringLiteral("right"));
    QCOMPARE(ConfigManager::alignmentToString(Qt::AlignCenter), QStringLiteral("center"));

    QCOMPARE(ConfigManager::stringToSizingMode(QStringLiteral("fixed")), SizingMode::Fixed);
    QCOMPARE(ConfigManager::stringToSizingMode(QStringLiteral("FIXED")), SizingMode::Fixed);
    QCOMPARE(ConfigManager::stringToSizingMode(QStringLiteral("responsive")), SizingMode::Responsive);
    QCOMPARE(ConfigManager::stringToSizingMode(QStringLiteral("other")), SizingMode::Responsive);

    QCOMPARE(ConfigManager::sizingModeToString(SizingMode::Fixed), QStringLiteral("fixed"));
    QCOMPARE(ConfigManager::sizingModeToString(SizingMode::Responsive), QStringLiteral("responsive"));
}

void TestConfigManager::testWorkingHoursLogic() {
    // Normal interval (e.g. 08:00 - 18:00)
    WorkingHours normal(8, 0, 18, 0);
    QCOMPARE(normal.formatRange(), QStringLiteral("08:00 - 18:00"));
    QVERIFY(!normal.isWorkingHour(QTime(7, 59, 59)));
    QVERIFY(normal.isWorkingHour(QTime(8, 0, 0)));
    QVERIFY(normal.isWorkingHour(QTime(12, 0, 0)));
    QVERIFY(normal.isWorkingHour(QTime(17, 59, 59)));
    QVERIFY(!normal.isWorkingHour(QTime(18, 0, 0)));
    QVERIFY(!normal.isWorkingHour(QTime(22, 0, 0)));

    // Cross-midnight interval (e.g. 20:00 - 06:00)
    WorkingHours crossMidnight(20, 0, 6, 0);
    QCOMPARE(crossMidnight.formatRange(), QStringLiteral("20:00 - 06:00"));
    QVERIFY(!crossMidnight.isWorkingHour(QTime(19, 59, 59)));
    QVERIFY(crossMidnight.isWorkingHour(QTime(20, 0, 0)));
    QVERIFY(crossMidnight.isWorkingHour(QTime(23, 59, 59)));
    QVERIFY(crossMidnight.isWorkingHour(QTime(0, 0, 0)));
    QVERIFY(crossMidnight.isWorkingHour(QTime(5, 59, 59)));
    QVERIFY(!crossMidnight.isWorkingHour(QTime(6, 0, 0)));
    QVERIFY(!crossMidnight.isWorkingHour(QTime(12, 0, 0)));

    // Empty interval (start == end)
    WorkingHours empty(9, 0, 9, 0);
    QVERIFY(!empty.isWorkingHour(QTime(9, 0, 0)));
}

QTEST_MAIN(TestConfigManager)
#include "TestConfigManager.moc"

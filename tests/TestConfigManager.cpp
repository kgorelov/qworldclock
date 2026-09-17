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
    QCOMPARE(def.window.width, 900);
    QCOMPARE(def.window.height, 650);
    QCOMPARE(def.clocks.size(), 1);
    QCOMPARE(def.clocks[0].id, QStringLiteral("clock-local"));
    QCOMPARE(def.clocks[0].caption, QStringLiteral("Local Time"));
    QCOMPARE(def.clocks[0].row, 0);
    QCOMPARE(def.clocks[0].col, 0);
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

    savedCfg.window.width = 1200;
    savedCfg.window.height = 800;
    savedCfg.window.x = 100;
    savedCfg.window.y = 150;
    savedCfg.window.maximized = true;

    savedCfg.clocks = {
        {QStringLiteral("c1"), QTimeZone::systemTimeZone(), QStringLiteral("Local"), 0, 0},
        {QStringLiteral("c2"), QTimeZone("Europe/London"), QStringLiteral("London Office"), 0, 1},
        {QStringLiteral("c3"), QTimeZone("Asia/Tokyo"), QStringLiteral("Tokyo HQ"), 1, 0}
    };

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

    QCOMPARE(loadedCfg.clocks[1].id, QStringLiteral("c2"));
    QCOMPARE(loadedCfg.clocks[1].caption, QStringLiteral("London Office"));
    QCOMPARE(loadedCfg.clocks[1].timeZone.id(), QByteArray("Europe/London"));
    QCOMPARE(loadedCfg.clocks[1].row, 0);
    QCOMPARE(loadedCfg.clocks[1].col, 1);

    QCOMPARE(loadedCfg.clocks[2].id, QStringLiteral("c3"));
    QCOMPARE(loadedCfg.clocks[2].caption, QStringLiteral("Tokyo HQ"));
    QCOMPARE(loadedCfg.clocks[2].timeZone.id(), QByteArray("Asia/Tokyo"));
    QCOMPARE(loadedCfg.clocks[2].row, 1);
    QCOMPARE(loadedCfg.clocks[2].col, 0);
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

QTEST_MAIN(TestConfigManager)
#include "TestConfigManager.moc"

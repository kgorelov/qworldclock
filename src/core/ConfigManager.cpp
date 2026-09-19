#include "core/ConfigManager.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimeZone>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <toml++/toml.h>
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <sstream>

namespace qworldclock {

ConfigManager::ConfigManager(const QString &configFilePath)
    : m_configFilePath(configFilePath.isEmpty() ? defaultFilePath() : configFilePath) {
}

QString ConfigManager::configFilePath() const {
    return m_configFilePath;
}

void ConfigManager::setConfigFilePath(const QString &path) {
    m_configFilePath = path.isEmpty() ? defaultFilePath() : path;
}

QString ConfigManager::defaultFilePath() {
    const QString xdg = qEnvironmentVariable("XDG_CONFIG_HOME");
    QString configDir;
    if (!xdg.isEmpty()) {
        configDir = xdg + QStringLiteral("/qworldclock");
    } else {
        configDir = QDir::homePath() + QStringLiteral("/.config/qworldclock");
    }
    return configDir + QStringLiteral("/qworldclock.cfg");
}

AppConfig ConfigManager::defaultConfig() {
    AppConfig cfg;
    cfg.version = QStringLiteral("1.0.0");
    cfg.sizingMode = QStringLiteral("responsive");
    cfg.fixedClockSize = 190;
    cfg.alignment = QStringLiteral("center");
    cfg.showMenuBar = true;
    cfg.showStatusBar = true;
    cfg.showSeconds = true;
    cfg.showDayNight = true;

    cfg.window.width = 900;
    cfg.window.height = 650;
    cfg.window.x = -1;
    cfg.window.y = -1;
    cfg.window.maximized = false;

    ClockItem localClock;
    localClock.id = QStringLiteral("clock-local");
    localClock.timeZone = QTimeZone::systemTimeZone();
    localClock.caption = QStringLiteral("Local Time");
    localClock.row = 0;
    localClock.col = 0;
    cfg.clocks.append(localClock);

    return cfg;
}

AppConfig ConfigManager::load() const {
    QFile file(m_configFilePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return defaultConfig();
    }

    const QByteArray rawData = file.readAll();
    file.close();

    const std::string content = rawData.toStdString();
    toml::table tbl;
    try {
        tbl = toml::parse(content);
    } catch (...) {
        return defaultConfig();
    }

    AppConfig cfg = defaultConfig();

    // [app]
    if (auto appTbl = tbl["app"].as_table()) {
        cfg.version = QString::fromStdString((*appTbl)["version"].value_or("1.0.0"));
        cfg.sizingMode = QString::fromStdString((*appTbl)["sizing_mode"].value_or("responsive"));
        cfg.fixedClockSize = static_cast<int>((*appTbl)["fixed_clock_size"].value_or(190));
        cfg.alignment = QString::fromStdString((*appTbl)["alignment"].value_or("center"));
        cfg.showMenuBar = (*appTbl)["show_menu_bar"].value_or(true);
        cfg.showStatusBar = (*appTbl)["show_status_bar"].value_or(true);
        cfg.showSeconds = (*appTbl)["show_seconds"].value_or(true);
        cfg.showDayNight = (*appTbl)["show_day_night"].value_or(true);

        const std::string startStr = (*appTbl)["working_hours_start"].value_or("08:00");
        const std::string endStr = (*appTbl)["working_hours_end"].value_or("18:00");
        const QTime st = QTime::fromString(QString::fromStdString(startStr), QStringLiteral("HH:mm"));
        const QTime et = QTime::fromString(QString::fromStdString(endStr), QStringLiteral("HH:mm"));
        cfg.workingHours.startTime = st.isValid() ? st : QTime(8, 0);
        cfg.workingHours.endTime = et.isValid() ? et : QTime(18, 0);
    }

    // [window]
    if (auto winTbl = tbl["window"].as_table()) {
        cfg.window.width = static_cast<int>((*winTbl)["width"].value_or(900));
        cfg.window.height = static_cast<int>((*winTbl)["height"].value_or(650));
        cfg.window.x = static_cast<int>((*winTbl)["x"].value_or(-1));
        cfg.window.y = static_cast<int>((*winTbl)["y"].value_or(-1));
        cfg.window.maximized = (*winTbl)["maximized"].value_or(false);
    }

    // [[clocks]]
    if (auto clocksArr = tbl["clocks"].as_array()) {
        QList<ClockItem> loadedClocks;
        for (auto &&node : *clocksArr) {
            if (auto clockTbl = node.as_table()) {
                ClockItem item;
                item.id = QString::fromStdString((*clockTbl)["id"].value_or(""));
                if (item.id.isEmpty()) {
                    continue;
                }

                const std::string tzStr = (*clockTbl)["timezone"].value_or("Local");
                if (tzStr == "Local" || tzStr.empty()) {
                    item.timeZone = QTimeZone::systemTimeZone();
                } else {
                    item.timeZone = QTimeZone(QByteArray::fromStdString(tzStr));
                    if (!item.timeZone.isValid()) {
                        item.timeZone = QTimeZone::systemTimeZone();
                    }
                }

                item.caption = QString::fromStdString((*clockTbl)["caption"].value_or(""));
                item.row = static_cast<int>((*clockTbl)["row"].value_or(0));
                item.col = static_cast<int>((*clockTbl)["col"].value_or(0));
                item.hasCustomWorkingHours = (*clockTbl)["has_custom_working_hours"].value_or(false);

                const std::string cStartStr = (*clockTbl)["working_hours_start"].value_or("08:00");
                const std::string cEndStr = (*clockTbl)["working_hours_end"].value_or("18:00");
                const QTime cSt = QTime::fromString(QString::fromStdString(cStartStr), QStringLiteral("HH:mm"));
                const QTime cEt = QTime::fromString(QString::fromStdString(cEndStr), QStringLiteral("HH:mm"));
                item.customWorkingHours.startTime = cSt.isValid() ? cSt : QTime(8, 0);
                item.customWorkingHours.endTime = cEt.isValid() ? cEt : QTime(18, 0);

                loadedClocks.append(item);
            }
        }

        if (!loadedClocks.isEmpty()) {
            cfg.clocks = loadedClocks;
        }
    }

    return cfg;
}

bool ConfigManager::save(const AppConfig &config) const {
    QFileInfo fi(m_configFilePath);
    QDir().mkpath(fi.absolutePath());

    toml::table root;

    // [app]
    root.insert_or_assign("app", toml::table{
        {"version", config.version.toStdString()},
        {"sizing_mode", config.sizingMode.toStdString()},
        {"fixed_clock_size", config.fixedClockSize},
        {"alignment", config.alignment.toStdString()},
        {"show_menu_bar", config.showMenuBar},
        {"show_status_bar", config.showStatusBar},
        {"show_seconds", config.showSeconds},
        {"show_day_night", config.showDayNight},
        {"working_hours_start", config.workingHours.startTime.toString(QStringLiteral("HH:mm")).toStdString()},
        {"working_hours_end", config.workingHours.endTime.toString(QStringLiteral("HH:mm")).toStdString()}
    });

    // [window]
    root.insert_or_assign("window", toml::table{
        {"width", config.window.width},
        {"height", config.window.height},
        {"x", config.window.x},
        {"y", config.window.y},
        {"maximized", config.window.maximized}
    });

    // [[clocks]]
    toml::array clocksArr;
    for (const auto &c : config.clocks) {
        const QString tzStr = (c.timeZone == QTimeZone::systemTimeZone())
                                  ? QStringLiteral("Local")
                                  : QString::fromUtf8(c.timeZone.id());
        clocksArr.push_back(toml::table{
            {"id", c.id.toStdString()},
            {"timezone", tzStr.toStdString()},
            {"caption", c.caption.toStdString()},
            {"row", c.row},
            {"col", c.col},
            {"has_custom_working_hours", c.hasCustomWorkingHours},
            {"working_hours_start", c.customWorkingHours.startTime.toString(QStringLiteral("HH:mm")).toStdString()},
            {"working_hours_end", c.customWorkingHours.endTime.toString(QStringLiteral("HH:mm")).toStdString()}
        });
    }
    root.insert_or_assign("clocks", std::move(clocksArr));

    std::stringstream ss;
    ss << root;
    const std::string tomlStr = ss.str();

    QFile file(m_configFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    const qint64 written = file.write(tomlStr.data(), static_cast<qint64>(tomlStr.size()));
    file.close();
    return written == static_cast<qint64>(tomlStr.size());
}

Qt::Alignment ConfigManager::stringToAlignment(const QString &str) {
    if (str.compare(QStringLiteral("left"), Qt::CaseInsensitive) == 0) {
        return Qt::AlignLeft;
    }
    if (str.compare(QStringLiteral("right"), Qt::CaseInsensitive) == 0) {
        return Qt::AlignRight;
    }
    return Qt::AlignCenter;
}

QString ConfigManager::alignmentToString(Qt::Alignment alignment) {
    if (alignment.testFlag(Qt::AlignLeft)) {
        return QStringLiteral("left");
    }
    if (alignment.testFlag(Qt::AlignRight)) {
        return QStringLiteral("right");
    }
    return QStringLiteral("center");
}

SizingMode ConfigManager::stringToSizingMode(const QString &str) {
    if (str.compare(QStringLiteral("fixed"), Qt::CaseInsensitive) == 0) {
        return SizingMode::Fixed;
    }
    return SizingMode::Responsive;
}

QString ConfigManager::sizingModeToString(SizingMode mode) {
    if (mode == SizingMode::Fixed) {
        return QStringLiteral("fixed");
    }
    return QStringLiteral("responsive");
}

} // namespace qworldclock

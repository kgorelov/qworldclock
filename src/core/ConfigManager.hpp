#pragma once

#include "core/GridModel.hpp"
#include "ui/ClockGridPanel.hpp"

#include <QList>
#include <QString>

namespace qworldclock {

struct WindowConfig {
    int width{900};
    int height{650};
    int x{-1};
    int y{-1};
    bool maximized{false};
};

struct AppConfig {
    QString version{QStringLiteral("1.0.0")};
    QString sizingMode{QStringLiteral("responsive")};
    int fixedClockSize{190};
    QString alignment{QStringLiteral("center")};
    bool showMenuBar{true};
    bool showStatusBar{true};
    bool showSeconds{true};
    bool showDayNight{true};
    WorkingHours workingHours{8, 0, 18, 0};

    WindowConfig window;
    QList<ClockItem> clocks;
};

class ConfigManager {
public:
    explicit ConfigManager(const QString &configFilePath = QString());
    ~ConfigManager() = default;

    [[nodiscard]] QString configFilePath() const;
    void setConfigFilePath(const QString &path);

    [[nodiscard]] static QString defaultFilePath();
    [[nodiscard]] static AppConfig defaultConfig();

    [[nodiscard]] AppConfig load() const;
    bool save(const AppConfig &config) const;

    // Helper conversion methods
    [[nodiscard]] static Qt::Alignment stringToAlignment(const QString &str);
    [[nodiscard]] static QString alignmentToString(Qt::Alignment alignment);

    [[nodiscard]] static SizingMode stringToSizingMode(const QString &str);
    [[nodiscard]] static QString sizingModeToString(SizingMode mode);

private:
    QString m_configFilePath;
};

} // namespace qworldclock

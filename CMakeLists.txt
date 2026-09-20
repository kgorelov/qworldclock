cmake_minimum_required(VERSION 3.22)
project(qworldclock VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt6 OPTIONAL_COMPONENTS Test)

find_package(tomlplusplus QUIET)
if(NOT tomlplusplus_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(tomlplusplus)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Core sources
set(CORE_SOURCES
    src/core/TimeEngine.cpp
    src/core/GridModel.cpp
    src/core/ConfigManager.cpp
)

# UI sources
set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/AnalogClockWidget.cpp
    src/ui/CaptionLabel.cpp
    src/ui/ClockCardWidget.cpp
    src/ui/ClockGridPanel.cpp
    src/ui/TimeZoneDialog.cpp
    src/ui/WorkingHoursDialog.cpp
)

# Main executable
add_executable(qworldclock
    src/main.cpp
    ${UI_SOURCES}
    ${CORE_SOURCES}
)

target_link_libraries(qworldclock
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        tomlplusplus::tomlplusplus
)

# Deploy Qt dependencies on Windows
if(WIN32 AND DEFINED ENV{CONDA_PREFIX})
    set(CONDA_BIN_DIR "$ENV{CONDA_PREFIX}/Library/bin")
    set(QT_BIN_DIR "$ENV{CONDA_PREFIX}/Library/lib/qt6/bin")
    set(QT_PLUGINS_DIR "$ENV{CONDA_PREFIX}/Library/lib/qt6/plugins")

    # Copy Qt DLLs
    add_custom_command(TARGET qworldclock POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_BIN_DIR}/Qt6Core.dll"
            "${QT_BIN_DIR}/Qt6Gui.dll"
            "${QT_BIN_DIR}/Qt6Widgets.dll"
            "$<TARGET_FILE_DIR:qworldclock>/"
        COMMENT "Copying Qt DLLs"
    )

    # Copy platform plugins (required for Qt apps to run)
    add_custom_command(TARGET qworldclock POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:qworldclock>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_PLUGINS_DIR}/platforms/qwindows.dll"
            "$<TARGET_FILE_DIR:qworldclock>/platforms/"
        COMMENT "Copying Qt platform plugins"
    )

    # Copy runtime dependencies (system libraries that Qt depends on)
    add_custom_command(TARGET qworldclock POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CONDA_BIN_DIR}/libpng16.dll"
            "${CONDA_BIN_DIR}/zlib.dll"
            "${CONDA_BIN_DIR}/pcre2-16.dll"
            "${CONDA_BIN_DIR}/zstd.dll"
            "${CONDA_BIN_DIR}/freetype.dll"
            "${CONDA_BIN_DIR}/harfbuzz.dll"
            "${CONDA_BIN_DIR}/libbz2.dll"
            "${CONDA_BIN_DIR}/double-conversion.dll"
            "${CONDA_BIN_DIR}/icuin78.dll"
            "${CONDA_BIN_DIR}/icuuc78.dll"
            "${CONDA_BIN_DIR}/icudt78.dll"
            "$<TARGET_FILE_DIR:qworldclock>/"
        COMMENT "Copying Qt runtime dependencies"
    )
endif()

# Enable testing if Qt6Test is found
if(TARGET Qt6::Test AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation rules
if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)

    install(
        TARGETS qworldclock
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(
        FILES resources/icons/qworldclock.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
    )
    install(
        FILES resources/qworldclock.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
endif()

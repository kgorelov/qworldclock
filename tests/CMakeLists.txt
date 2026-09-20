find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)

add_executable(test_grid_model
    TestGridModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/GridModel.cpp
)

target_include_directories(test_grid_model PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(test_grid_model PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME GridModelTest COMMAND test_grid_model)

# TimeEngine test
add_executable(test_time_engine
    TestTimeEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/TimeEngine.cpp
)

target_include_directories(test_time_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(test_time_engine PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME TimeEngineTest COMMAND test_time_engine)

# Config Manager test
add_executable(test_config_manager
    TestConfigManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/ConfigManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/GridModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/TimeEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/AnalogClockWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/CaptionLabel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ClockCardWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ClockGridPanel.cpp
)

target_include_directories(test_config_manager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(test_config_manager PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
    tomlplusplus::tomlplusplus
)

add_test(NAME ConfigManagerTest COMMAND test_config_manager)

# Window UI test
add_executable(test_window
    TestWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/ConfigManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/GridModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/TimeEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/AnalogClockWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/CaptionLabel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ClockCardWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ClockGridPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/MainWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/TimeZoneDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/WorkingHoursDialog.cpp
)

target_include_directories(test_window PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(test_window PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
    tomlplusplus::tomlplusplus
)

add_test(NAME WindowTest COMMAND test_window)

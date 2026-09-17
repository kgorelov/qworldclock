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
)

# UI sources
set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/AnalogClockWidget.cpp
    src/ui/CaptionLabel.cpp
    src/ui/ClockCardWidget.cpp
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

# Enable testing if Qt6Test is found
if(TARGET Qt6::Test AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    enable_testing()
    add_subdirectory(tests)
endif()

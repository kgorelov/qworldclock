find_package(Qt6 REQUIRED COMPONENTS Core Test)

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

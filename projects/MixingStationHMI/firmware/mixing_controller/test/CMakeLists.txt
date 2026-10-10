cmake_minimum_required(VERSION 3.22)

set(
    EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../components"
)

set(
    TEST_COMPONENTS
    "controller_core"
    CACHE STRING
    "Components included in the firmware test application"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

idf_build_set_property(MINIMAL_BUILD ON)

project(mixing_controller_tests)

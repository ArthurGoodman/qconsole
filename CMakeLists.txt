cmake_minimum_required(VERSION 3.0.0)
project(qconsole)

################################################################################
# CMake options
################################################################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

################################################################################
# Compiler settings
################################################################################

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

set(FLAGS "-Wall -Wextra -Wunused-variable -Wno-comment")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")

################################################################################
# Libraries
################################################################################

# Qt5
find_package(Qt5 COMPONENTS
    Widgets
    REQUIRED)

################################################################################
# Targets
################################################################################

set(QCONSOLE ${PROJECT_NAME})

add_subdirectory(src)

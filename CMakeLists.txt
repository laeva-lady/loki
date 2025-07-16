cmake_minimum_required(VERSION 3.10)
project(loki)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# Collect all source files
file(GLOB_RECURSE SOURCES
    source/*.cpp
)

add_executable(loki ${SOURCES})
target_include_directories(loki PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_libraries(loki ${GTKMM_LIBRARIES})

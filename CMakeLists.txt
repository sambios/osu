cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 14)
set(UTILITY_TOP ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${UTILITY_TOP})

add_library(osu osu_timer.cpp)

add_executable(osu_timer_unittest osu_timer_unittest.cpp)
target_link_libraries(osu_timer_unittest osu)
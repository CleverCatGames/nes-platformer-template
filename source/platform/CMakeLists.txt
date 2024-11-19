cmake_minimum_required(VERSION 3.11)

add_library(platform_lib
    bank.s
    donut.s
    rand.s
    sleep.s
    util.s

    mapper/${MAPPER}.s
)

add_subdirectory(util)


if(CMAKE_SYSTEM_NAME STREQUAL "NES")
    add_subdirectory(nes)
    target_link_libraries(platform_lib platform_util_lib nes_lib)

endif()


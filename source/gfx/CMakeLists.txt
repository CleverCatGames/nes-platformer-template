cmake_minimum_required(VERSION 3.11)

add_library(gfx_lib
    graphics.c
    general.c

    # zone specific gfx
    example.c
)

target_link_libraries(gfx_lib platform_lib)

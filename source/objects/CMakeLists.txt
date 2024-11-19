cmake_minimum_required(VERSION 3.11)

add_subdirectory(util)

add_library(objects_lib
    enemy.c
    npc.c
)

target_link_libraries(objects_lib util_lib)


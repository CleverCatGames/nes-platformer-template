cmake_minimum_required(VERSION 3.11)

add_subdirectory(platform)
add_subdirectory(sound)

add_library(source_lib
    # asm
    button.s     # button press/release
    megatile.s   # level megatiles
    zeropage.s   # zeropage variables
    # c
    level.c      # level load and scroll
    object.c     # object logic
    player.c     # player logic
    ppu.c        # PPU helpers
    trigger.c    # full screen AABB
    variables.c  # BSS variables
)

add_subdirectory(objects)
add_subdirectory(gfx)

target_link_libraries(source_lib
    platform_lib  # NES specific functions
    sound_lib     # sound/music with FamiStudio
    gfx_lib       # graphics functions

    objects_lib   # object code
)

# Include debug library if in debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(debug)
    target_link_libraries(source_lib debug_lib)
endif()


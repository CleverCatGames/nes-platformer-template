cmake_minimum_required(VERSION 3.11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(NOT DEFINED CMAKE_SYSTEM_NAME)
    set(CMAKE_SYSTEM_NAME NES)
endif()

set(CMAKE_C_STANDARD 99)

#message("SYSTEM NAME ${CMAKE_SYSTEM_NAME}")

set(ASSET_DIR ${CMAKE_SOURCE_DIR}/build/assets)

set(CMAKE_ASM_CC65_FLAGS "--bin-include-dir ${ASSET_DIR}")
set(CMAKE_C_FLAGS "-Oi --add-source -DDTE")

IF(CMAKE_BUILD_TYPE STREQUAL "Assert")
    set(CMAKE_C_FLAGS "-DASSERT ${CMAKE_C_FLAGS}")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Profile")
    set(CMAKE_C_FLAGS "-DPROFILER ${CMAKE_C_FLAGS}")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(NAME game)

set(APP ${NAME}.nes)
set(MAPPER mmc1)
project(${NAME} C ASM_CC65)

if(MAPPER STREQUAL "mmc1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPRGRAM")
endif()

set(LINKER_FLAGS  "--config ${CMAKE_SOURCE_DIR}/config/${MAPPER}.cfg")

# create map/dbg/lbl files
set(LINKER_FLAGS "${LINKER_FLAGS} --mapfile ${NAME}.map --dbgfile ${NAME}.dbg -Ln ${NAME}.lbl")

set(CMAKE_EXE_LINKER_FLAGS ${LINKER_FLAGS})
set(CMAKE_MODULE_LINKER_FLAGS ${LINKER_FLAGS})

include_directories(${ASSET_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/include)
add_subdirectory(source)

add_executable(${APP} source/main.c)
target_link_libraries(${APP} source_lib)


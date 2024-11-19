# cc65 toolchain for CMake
# Copyright (C) 2021 Tentei Ltd.
# DISTRIBUTED AS PUBLIC DOMAIN. No restrictions apply.

set(CMAKE_ASM_CC65_SOURCE_FILE_EXTENSIONS asm;S;s)
set(CMAKE_ASM_CC65_COMPILER_ARG1 "")
set(CMAKE_ASM_CC65_DEFINE_FLAG "-D ")
set(CMAKE_ASM_CC65_FLAGS_DEBUG_INIT "-g -DDEBUG")
set(CMAKE_ASM_CC65_VERBOSE_FLAG "-v" )
set(CMAKE_DEPFILE_FLAGS_ASM_CC65 "--create-dep <DEP_FILE>")

set(CMAKE_ASM_CC65_COMPILER_AR ar65 CACHE PATH "cc65 archiver")
set(CMAKE_LINKER ld65 CACHE PATH "cc65 linker")

set(CMAKE_ASM_CC65_COMPILE_OBJECT
"<CMAKE_ASM_CC65_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")

set(CMAKE_ASM_CC65_LINK_EXECUTABLE
"<CMAKE_LINKER> <LINK_FLAGS> <LINK_LIBRARIES> <OBJECTS> -o <TARGET>")

SET(CMAKE_ASM_CC65_CREATE_STATIC_LIBRARY
"<CMAKE_AR> r <TARGET> <LINK_FLAGS> <OBJECTS>")

set(ASM_DIALECT "_CC65")
include(CMakeASMInformation)
set(CMAKE_INCLUDE_FLAG_ASM_CC65 "-I ")
set(ASM_DIALECT)


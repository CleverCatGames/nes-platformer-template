# cc65 toolchain for CMake
# Copyright (C) 2021 Tentei Ltd.
# DISTRIBUTED AS PUBLIC DOMAIN. No restrictions apply.

include_guard()

if(CMAKE_SYSTEM_NAME)
	if(CMAKE_SYSTEM_NAME MATCHES NES)
		set(_SYSTEM nes)

	elseif(CMAKE_SYSTEM_NAME MATCHES SNES)
		set(_SYSTEM snes)
		set(_CPU 65816)

	endif()
endif()

if(NOT _SYSTEM)
	set(_SYSTEM none)
endif()

if(NOT _CPU)
	set(_CPU 6502)
endif()

add_compile_options(--cpu ${_CPU} --target ${_SYSTEM})

unset(_SYSTEM)
unset(_CPU)

set(CMAKE_AR /usr/bin/ar65) # WHY?
set(CMAKE_LINKER /usr/bin/ld65) # WHY?

find_program(CMAKE_AR ar65 DOC "cc65 archiver")
set(CMAKE_${lang}_COMPILER_AR ${CMAKE_AR} CACHE FILEPATH "cc65 archiver" FORCE)
find_program(CMAKE_LINKER ld65 DOC "cc65 linker")

set(CMAKE_${lang}_RANLIB "" CACHE FILEPATH "" FORCE)


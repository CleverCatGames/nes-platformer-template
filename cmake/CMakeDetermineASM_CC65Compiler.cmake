# cc65 toolchain for CMake
# Copyright (C) 2021 Tentei Ltd.
# DISTRIBUTED AS PUBLIC DOMAIN. No restrictions apply.

if(NOT CMAKE_ASM_CC65_COMPILER)
	SET(_COMPILER_LIST ca65)

	find_program(
		CMAKE_ASM_CC65_COMPILER
		NAMES ${_COMPILER_LIST}
		DOC "cc65 assembler")

	unset(_COMPILER_LIST)

	if(CMAKE_ASM_CC65_COMPILER)
		set(CMAKE_ASM_CC65_COMPILER_ID CA65 CACHE PATH "cc65 assembler ID")

		execute_process(
			COMMAND ${CMAKE_ASM_CC65_COMPILER} --version
			OUTPUT_VARIABLE _OUTPUT1
			ERROR_VARIABLE _OUTPUT2)

		if("${_OUTPUT1};${_OUTPUT2}" MATCHES ".*ca65 V([^\n]*)-.*")
			set(CMAKE_ASM_CC65_COMPILER_VERSION "${CMAKE_MATCH_1}" CACHE PATH "cc65 assembler version")
		endif()

		unset(_OUTPUT1)
		unset(_OUTPUT2)

		set(lang ASM_CC65)
		include(Compiler/CC65)
	endif()
endif()

set(ASM_DIALECT "_CC65")
include(CMakeDetermineASMCompiler)
set(ASM_DIALECT)

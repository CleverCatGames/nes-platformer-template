# cc65 toolchain for CMake
# Copyright (C) 2021 Tentei Ltd.
# DISTRIBUTED AS PUBLIC DOMAIN. No restrictions apply.

if(NOT CMAKE_C_COMPILER)
	SET(_COMPILER_LIST cc65)

	find_program(
		CMAKE_C_COMPILER
		NAMES ${_COMPILER_LIST}
		DOC "cc65 C compiler")

	unset(_COMPILER_LIST)

	if(CMAKE_C_COMPILER)
		set(CMAKE_C_COMPILER_ID CC65 CACHE PATH "C compiler ID")

		execute_process(
			COMMAND ${CMAKE_C_COMPILER} --version
			OUTPUT_VARIABLE _OUTPUT1
			ERROR_VARIABLE _OUTPUT2)

		if("${_OUTPUT1};${_OUTPUT2}" MATCHES ".*cc65 V([^\n]*)-.*")
			set(CMAKE_C_COMPILER_VERSION "${CMAKE_MATCH_1}" CACHE PATH "C compiler version")
		endif()

		unset(_OUTPUT1)
		unset(_OUTPUT2)

		set(CMAKE_C_ABI_COMPILED YES)
		set(CMAKE_C_COMPILER_WORKS YES)
	endif()
endif()

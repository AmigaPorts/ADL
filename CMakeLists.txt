cmake_minimum_required(VERSION 2.8.5)

FIND_PROGRAM(VASM vasmm68k_mot)
if (VASM)
	# Hack to use VASM to compile the assembly code
	set(CMAKE_ASM_COMPILER_ENV_VAR ${CMAKE_ASM_COMPILER_ENV_VAR}" ")
	set(CMAKE_ASM_COMPILER ${VASM})
	set(CMAKE_ASM_SOURCE_FILE_EXTENSION ".s")
	set(CMAKE_ASM_OUTPUT_EXTENSION ".o")
	set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")
	set(CMAKE_ASM_FLAGS "-m68030 -I${CMAKE_SYSROOT}/m68k-amigaos/ndk-include -Fhunk")
else ()
	message(ERROR "Could not find vasmm68k_mot! It is required to compile this library!")
endif ()

	project(ADL ASM C)

	set(
			SOURCES
			src/control.c
			src/timer.c
			src/video.c
	)

	if (WARPOS)
		list(
				APPEND
				SOURCES
				src/ppc/
		)
	elseif (AMIGAOS3)
		list(
				APPEND
				SOURCES
				src/68k/c2p1x1_8_c5_bm.s
				src/68k/c2p1x1_8_c5_bm_040.s
		)
	endif ()

	set(
			HEADERS
			include/SDL.h
			include/control.h
			include/joysticks.h
			include/timer.h
			include/video.h
	)

	include_directories(
			${CMAKE_CURRENT_SOURCE_DIR}/include
	)

	set(${PROJECT_NAME}_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)

	add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

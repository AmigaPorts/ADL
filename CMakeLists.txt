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

	project(ADL ASM C)

	set(
			SOURCES
			c2p1x1_8_c5_bm.s
			c2p1x1_8_c5_bm_040.s
			control.c
			timer.c
			video.c
	)

	set(
			HEADERS
			SDL.h
			control.h
			joysticks.h
			timer.h
			video.h
	)

	include_directories(
			${CMAKE_CURRENT_SOURCE_DIR}
	)

	set(${PROJECT_NAME}_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

	add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
else ()
	message(ERROR "Could not find vasmm68k_mot! It is required to compile this library!")
endif ()
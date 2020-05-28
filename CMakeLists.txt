cmake_minimum_required(VERSION 2.8.5)

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
	enable_language(ASM_VASM)
	list(
			APPEND
			SOURCES
			src/68k/c2p1x1_8_c5_bm.asm
			src/68k/c2p1x1_8_c5_bm_040.asm
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
#set(CMAKE_C_FLAGS "")
#set(CMAKE_LD_FLAGS "")

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

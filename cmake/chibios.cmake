

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/generated/chibios_file_generated.txt ChibiOS_entries)


string(REPLACE "make: Entering directory '${CMAKE_CURRENT_LIST_DIR}'" ""  ChibiOS_entries ${ChibiOS_entries})
string(REGEX REPLACE "Makefile:[0-9]*:" ""  ChibiOS_entries ${ChibiOS_entries})
string(REPLACE "***" ""  ChibiOS_entries ${ChibiOS_entries})
string(REPLACE ".  Stop." ""  ChibiOS_entries ${ChibiOS_entries})
string(REPLACE " " ";" ChibiOS_entries ${ChibiOS_entries})


FOREACH(entry ${ChibiOS_entries})
	if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${entry}")
		if(IS_DIRECTORY  "${CMAKE_CURRENT_LIST_DIR}/${entry}")
			LIST (APPEND ChibiOS_INCLUDE_DIRS ${entry})
			MESSAGE(DIR "\t${entry}")
		else()
			LIST (APPEND ChibiOS_SOURCES ${entry})
			MESSAGE(FILE "\t${entry}")
		endif()
	else()
		MESSAGE(NOT FOUND "\t${entry}")
	endif()
ENDFOREACH()

IF(${CHIBIOS_DEBUG})
	LIST (APPEND PROJECT_SOURCES
			${CHIBIOS_ROOT}/os/rt/src/chdebug.c
			${CHIBIOS_ROOT}/os/rt/src/chtrace.c
			${CHIBIOS_ROOT}/os/rt/src/chstats.c
			)
ENDIF()

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L\"${CHIBIOS_ROOT}/os/common/startup/ARMCMx/compilers/GCC/ld\"")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--defsym=__process_stack_size__=${CHIBIOS_PROCESS_STACK_SIZE}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--defsym=__main_stack_size__=${CHIBIOS_MAIN_STACK_SIZE}")

LIST(REMOVE_DUPLICATES ChibiOS_INCLUDE_DIRS)
LIST(REMOVE_DUPLICATES ChibiOS_SOURCES)



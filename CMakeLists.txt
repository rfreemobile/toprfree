cmake_minimum_required(VERSION 3.0)
project (toprfree)

if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Debug")
	set(CMAKE_BUILD_TYPE "Debug")
endif()


if(MINGW)
	add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
endif()

if(WIN32 OR CYGWIN)
	add_definitions(-DUNICODE)
	add_definitions(-D_UNICODE)
	add_definitions(-DWINVER=0x0601)
	add_definitions(-D_WIN32_WINNT=0x0601)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOMINMAX)
endif()


########################### BOOST ###########################
### boost - part 1 - just check version and decide does it affect our flags
set (Boost_USE_MULTITHREADED ON)
if (BUILD_STATIC_MAINPROGRAM)
	set (Boost_USE_STATIC_RUNTIME ON)
	set (Boost_USE_STATIC_LIBS ON)
	set (Boost_USE_DEBUG_RUNTIME OFF)
endif()

if (MSVC)
	set(BOOST_LIBS_LIST system thread date_time)
elseif (MINGW)
	set(BOOST_LIBS_LIST system program_options date_time)
else()
	set(BOOST_LIBS_LIST system program_options thread date_time)
endif()
find_package(Boost 1.49.0 REQUIRED COMPONENTS ${BOOST_LIBS_LIST})

if(NOT Boost_FOUND)
	message("\nBOOST NOT FOUND!")
	if (MSVC)
		message(STATUS "\nNote (MSVC): \nPlease download and install package (or later):")
		message(STATUS "    https://sourceforge.net/projects/boost/files/boost-binaries/ boost_1_61_0-msvc-14.0-64.exe \n")
		message(STATUS "Add following argument for cmake (default install location):")
		message(STATUS "    \"-DBOOST_ROOT=C:/local/boost_1_61_0 -DBOOST_LIBRARYDIR=C:/local/boost_1_61_0/lib64-msvc-14.0/\"\n\n")
	endif()
	message(FATAL_ERROR "BOOST REQUIRED AND NOT FOUND, NOW STOPPING")
endif()

message(STATUS "Boost version variable is: ${Boost_VERSION}")
set(Boost_IS_BROKEN_UB OFF)
if ((Boost_VERSION GREATER 105399) AND (Boost_VERSION LESS 105700)) # 1.54 is broken, fix in 1.57
	set(Boost_IS_BROKEN_UB ON)
	message(WARNING "Warning: this Boost version is known to have a bug: https://svn.boost.org/trac10/ticket/12570")
endif()

if (Boost_IS_BROKEN_UB)
	set(TOOLS_ARE_BROKEN ON)
	set(TOOLS_ARE_BROKEN_UB ON)
	message(WARNING "Warning: this Boost version has bug(s) - that will theoretically cause C++ UB (bug in our program), use other one if possible, we can not guarantee security of program built here!")
endif()

######################################################################
########################### COMPILER FLAGS ###########################
######################################################################

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_VERSION VERSION_LESS 3.1)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++${CMAKE_CXX_STANDARD}") # [[version]] needed for Debian 8
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	set(WARN_PEDANTIC_FLAGS "-Wall -Wextra -pedantic -Wnoexcept -Wstrict-null-sentinel -Wlogical-op")
	set(WARN_PEDANTIC_FLAGS "${WARN_PEDANTIC_FLAGS} -Wreturn-type")
	set(WARN_PEDANTIC_FLAGS "${WARN_PEDANTIC_FLAGS} -Werror -Wfatal-errors")
	set(WARN_PEDANTIC_FLAGS_FOR_C "-Wall") # mostly used by dependencies
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
	if(APPLE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -stdlib=libc++")
	endif()

	### clang sanitizers ###
	option(SANITIZER_THREAD "Enable TSAN - clang thread sanitizer" OFF)
	option(SANITIZER_ADDRESS "Enable ASAN - clang address sanitizer" OFF)
	option(SANITIZER_UNDEFINED_BEHAVIOR "Enable UBSAN - clang undefined behavior sanitizer" OFF)
	set(SANITIZER_DETAILED "-fno-omit-frame-pointer -fno-optimize-sibling-calls")
	if(SANITIZER_THREAD)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-sanitize-recover=thread ${SANITIZER_DETAILED}")
		add_definitions(-DFLAVOUR_TSAN_FULL) # we have full TSAN and can rely on it
	endif()
	if(SANITIZER_ADDRESS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-sanitize-recover=address ${SANITIZER_DETAILED}")
	endif()
	if(SANITIZER_UNDEFINED_BEHAVIOR)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fsanitize=integer ${SANITIZER_DETAILED}")
		if (NOT TOOLS_ARE_BROKEN_UB)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-sanitize-recover=undefined")
			add_definitions(-DFLAVOUR_UBSAN_FULL) # we have full UBSAN and can rely on it
		else()
			message(WARNING "Warning: we have to partially disable UBSAN check (that is: program will continue to run, ignoring an UB!) because the tools used are broken and produce UBs.")
			add_definitions(-DFLAVOUR_UBSAN_ONLY_REPORTS) # we have partiall UBSAN and can not rely on it's results
		endif()
	endif()
	# https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
	set(WARN_PEDANTIC_FLAGS "-Wall -Wextra -pedantic -Wno-unused-command-line-argument -Wthread-safety")
	set(WARN_PEDANTIC_FLAGS "${WARN_PEDANTIC_FLAGS} -Werror -Wfatal-errors")

	set(WARN_PEDANTIC_FLAGS "${WARN_PEDANTIC_FLAGS} -Wreturn-type")
	set(WARN_PEDANTIC_FLAGS_FOR_C "-Wall -Wno-unused-command-line-argument") # mostly used by dependencies
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(WARN_PEDANTIC_FLAGS "/W4")
	string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
else()
	message(fatal_error "Compiler not supported")
endif()


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARN_PEDANTIC_FLAGS_FOR_C} ${STATIC_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARN_PEDANTIC_FLAGS} ${STATIC_FLAGS} ${COVERAGE_CXX_FLAG}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MINGW_LINK_FLAGS}")

# more directories of my main code

######################################################################
########################### DEPENDS ##################################
######################################################################

########################### BOOST ###########################
### boost - part2
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

IF(DEFINED USE_BOOST_MULTIPRECISION_DEFAULT)
	SET(USE_BOOST_MULTIPRECISION_DEFAULT ${USE_BOOST_MULTIPRECISION_DEFAULT} CACHE INTERNAL "")
ELSE()
	SET(USE_BOOST_MULTIPRECISION_DEFAULT ON CACHE INTERNAL "")
ENDIF()

option(USE_BOOST_MULTIPRECISION "Can we use boost::multiprecision - it was broken https://svn.boost.org/trac/boost/ticket/12181" ${USE_BOOST_MULTIPRECISION_DEFAULT})

if(USE_BOOST_MULTIPRECISION)
	message(STATUS "USE_BOOST_MULTIPRECISION is normal (on)")
	add_definitions(-DUSE_BOOST_MULTIPRECISION=1)
else()
	message(STATUS "USE_BOOST_MULTIPRECISION is DISABLED")
	add_definitions(-DUSE_BOOST_MULTIPRECISION=0)
endif()


########################### SOURCE FILES ###########################
include_directories( src/
	src/pfp-local/
)

file(GLOB SOURCES_GROUP_MAINPROGRAM src/*
	src/sensor/*
	src/mylib/*
	src/ui/*
)
# ^-- TODO perhaps separate mylib a bit?

list(REMOVE_ITEM SOURCES_GROUP_MAINPROGRAM
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

message(STATUS "Sorting sources (from GLOB) - must be sorted for Gitian to be deterministic")
list(SORT SOURCES_GROUP_MAINPROGRAM)


########################### OPTIONAL LIBRARIES ###########################

# perhaps use CAP in future?

# set(LIB_OPTIONAL_CAPNG "")
# if (UNIX AND NOT CYGWIN AND NOT APPLE)
# 	set(LIB_OPTIONAL_CAPNG "cap-ng")
# endif ()

######################################################################
######################################################################
######################################################################

########################### TARGETS ###########################

add_executable(toprfree src/main.cpp ${SOURCES_GROUP_MAINPROGRAM})

set(LIBS_COMMON
	${Boost_LIBRARIES} ${BOOST_LOCALE_LINK_LIB} ${BOOST_THREAD_LINK_LIB}
	${PTHREAD_LIB}
	${LIB_OPTIONAL_CAPNG}
	${MINGW_LIBRARIES} ${MSVC_LIRARIES}
	ncurses
)

target_link_libraries(toprfree
	${LIBS_COMMON}
	)

#tests
# file(GLOB TEST_SOURCES "src/test/*.cpp")
# list(SORT TEST_SOURCES)

# add_executable(unittests.elf ${SOURCES_GROUP_MAINPROGRAM} ${SOURCES_GROUP_TOOLS_NETMODEL} ${SOURCES_GROUP_CRYPTO} ${SOURCES_GROUP_RPC} ${TEST_SOURCES})
#target_compile_definitions(unittests.elf PUBLIC -DUSE_MOCK)
#target_link_libraries(unittests.elf
#	${LIBS_COMMON}
#	gtest gmock
#)


########################### FINALIZE OPTIONS #########################

if (TOOLS_ARE_BROKEN)
	# pass on to the C++ option informing that tools are broken
	add_definitions(-DTOOLS_ARE_BROKEN=1)
endif()

message(STATUS "C++ Compiler ID:               ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C Flags:                       ${CMAKE_C_FLAGS}")
message(STATUS "C++ Flags:                     ${CMAKE_CXX_FLAGS}")
message(STATUS "C++ link flags:                ${CMAKE_CXX_LINK_FLAGS}")
message(STATUS "Build type:                    ${CMAKE_BUILD_TYPE}")
message(STATUS "Debug flags:                   ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "Release flags:                 ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Release with debug info flags: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "Warn pedantic flags:           ${WARN_PEDANTIC_FLAGS}")

########################### CUSTOM TARGETS ###########################

add_custom_target(Release
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
		COMMAND make)


if(NOT MINGW AND NOT WIN32)

	add_custom_target(run
			COMMAND "./toprfree"
			DEPENDS ./toprfree
			WORKING_DIRECTORY ./)


if(false)
	add_custom_command(
		TARGET tunserver.elf
		POST_BUILD
		COMMAND rm -f "./nocap-tunserver.elf"
		COMMAND chmod g-wx,o-wx tunserver.elf
		COMMAND cp tunserver.elf nocap-tunserver.elf
		# remove and copy again scripts: run
		COMMAND rm -f "./rundevel.sh"
		COMMAND cp "${CMAKE_SOURCE_DIR}/src/scripts/run/rundevel.sh" "${CMAKE_BINARY_DIR}/" || echo "Can not copy file"
		# remove and copy again scripts: qa/run-safe
		# remove here all files copied [[list_qa_run_safe_files]]
		COMMAND rm -f "./run-safe-mem"
		COMMAND rm -f "./run-safe-thread-ub"
		COMMAND rm -f "./valgrind_log_watch"
		COMMAND rm -f "./valgrind_with_log_kill"
		COMMAND cp "${CMAKE_SOURCE_DIR}/qa/run-safe/*" "${CMAKE_BINARY_DIR}/" || echo skip

		WORKING_DIRECTORY ./)
endif()

endif()

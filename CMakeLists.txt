cmake_minimum_required(VERSION 3.0)

project(remote-shared-controller)

find_package(Threads REQUIRED)

include_directories(
  src/network
  src/event_interface
  src/core
  ) 
 
option(INCLUDE_TEST OFF)
option(COVERAGE OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "-fdiagnostics-color=always ${CMAKE_CXX_FLAGS}")
	if(CMAKE_BUILD_TYPE MATCHES Release)
	  message(STATUS "Release")
	  set(CMAKE_CXX_FLAGS "-O2 -O3 ${CMAKE_CXX_FLAGS}")
	  set(CMAKE_C_FLAGS "-O2 -O3 ${CMAKE_CXX_FLAGS}")
	else()
	  message(STATUS "Debug")
	  set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra ${CMAKE_CXX_FLAGS}")
	  set(CMAKE_C_FLAGS "-g -O0 -Wall -Wextra ${CMAKE_CXX_FLAGS}")
	endif()

	if(COVERAGE)
	  message(STATUS "Coverage enabled")
	  set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage")
	  set(CMAKE_C_FLAGS "-fprofile-arcs -ftest-coverage")
	  SET(ENV{LDFLAGS} "-fprofile-arcs -ftest-coverage $ENV{LDFLAGS}")
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  file(GLOB_RECURSE event_interface_sources src/event_interface/*_win.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
  file(GLOB_RECURSE event_interface_sources src/event_interface/*_linux.c)
else()
  message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not handled")
endif()

add_library(event_interface ${event_interface_sources})

file(GLOB_RECURSE network_sources src/network/*.c)
add_library(network ${network_sources})

file(GLOB_RECURSE core_sources src/core/*.cpp)
file(GLOB_RECURSE core_headers src/core/*.hpp)
add_executable(${CMAKE_PROJECT_NAME} ${core_sources} ${core_headers})
target_link_libraries(${CMAKE_PROJECT_NAME}
  event_interface
  network
  Threads::Threads
  )

if(INCLUDE_TEST)
  include(CTest)
  add_subdirectory(src/tests)
else()
  message(STATUS "Tests will not be included")
endif()

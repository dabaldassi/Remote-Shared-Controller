cmake_minimum_required(VERSION 3.1.3)

project(remote-shared-controller)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  src/network
  src/controller
  src/core
  src/rsclocal_com
  src/rsccli
  src/common
  src/rscgui
  src/
  ) 
 
option(INCLUDE_TEST OFF)
option(COVERAGE OFF)
option(NOCURSOR OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

find_package(Threads REQUIRED)


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "-fdiagnostics-color=always ${CMAKE_CXX_FLAGS}")
	if(CMAKE_BUILD_TYPE MATCHES Release)
	  message(STATUS "Release")
	  set(CMAKE_CXX_FLAGS "-O2 -O3 ${CMAKE_CXX_FLAGS}")
	  set(CMAKE_C_FLAGS "-O2 -O3 ${CMAKE_C_FLAGS}")
	else()
	  message(STATUS "Debug")
	  set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra ${CMAKE_CXX_FLAGS}")
	  set(CMAKE_C_FLAGS "-g -O0 -Wall -Wextra ${CMAKE_C_FLAGS}")
	endif()

	if(COVERAGE)
	  message(STATUS "Coverage enabled")
	  set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage")
	  set(CMAKE_C_FLAGS "-fprofile-arcs -ftest-coverage")
	  SET(ENV{LDFLAGS} "-fprofile-arcs -ftest-coverage $ENV{LDFLAGS}")
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  find_package(pthreads REQUIRED)

  file(GLOB controller_sources src/controller/windows/*.cpp src/controller/translate.c)
  file(GLOB network_src src/network/windows/*.c)
  file(GLOB rsclocal_com_src src/rsclocal_com/windows/*.cpp)

  include_directories(
      ${PThreads4W_INCLUDE_DIR}
      external/npcap/Include
  )

  set(NETWORK_LIB ${PThreads4W_LIBRARY} wpcap.lib Packet.lib)

  file(COPY external/npcap/Lib/x64/wpcap.lib external/npcap/Lib/x64/Packet.lib DESTINATION ${CMAKE_BINARY_DIR})

elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)

  file(GLOB_RECURSE rsclocal_com_src src/rsclocal_com/linux/*.cpp)
  file(GLOB network_src src/network/linux/*.c)
  set(NETWORK_LIB pthread)

  if(NOT NOCURSOR)
    find_package(X11 COMPONENTS Xfixes)

    if( X11_FOUND AND X11_Xfixes_FOUND)
      message(STATUS "X11 is found, compiling with X11 features")
      set(controller_libs ${X11_LIBRAIRIES} ${X11_X11_LIB} ${X11_Xfixes_LIB})
      file(GLOB_RECURSE controller_sources src/controller/linux/*.c src/controller/translate.c)
    else()
      message(WARNING "X11 is not found, assuming there is no graphic server\n"
	 "You must install X11 and X11 fixes to use X11 features")
      add_compile_options(-DNO_CURSOR)
      file(GLOB_RECURSE controller_sources
	src/controller/translate.c
	src/controller/linux/controller_linux.c)
    endif()
  else()
    file(GLOB_RECURSE controller_sources
      src/controller/linux/controller_linux.c
      src/controller/translate.c)
    add_compile_options(-DNO_CURSOR)
  endif()
  
else()
  message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not handled")
endif()

add_library(controller ${controller_sources})
target_link_libraries(controller ${controller_libs})

file(GLOB rsclocal_com_header src/rsclocal_com/*.hpp)
file(GLOB rsclocal_com_sources src/rsclocal_com/*.cpp)
list(APPEND rsclocal_com_sources ${rsclocal_com_src})
add_library(rsclocal_com ${rsclocal_com_sources} ${rsclocal_com_header})

file(GLOB network_sources src/network/*.c)
list(APPEND network_sources ${network_src})
add_library(network ${network_sources})
target_link_libraries(network ${NETWORK_LIB})

file(GLOB_RECURSE common_sources src/common/*.cpp)
file(GLOB_RECURSE common_headers src/common/*.hpp)
add_library(common ${common_sources} ${common_headers})
target_link_libraries(common controller rsclocal_com network)

file(GLOB_RECURSE core_sources src/core/*.cpp)
file(GLOB_RECURSE core_headers src/core/*.hpp)
add_executable(${CMAKE_PROJECT_NAME} ${core_sources} ${core_headers})
target_link_libraries(${CMAKE_PROJECT_NAME}
  controller
  network
  common
  rsclocal_com
  Threads::Threads
  )

file(GLOB_RECURSE rsccli_sources src/rsccli/*.cpp)
file(GLOB_RECURSE rsccli_headers src/rsccli/*.hpp)
add_executable(rsccli ${rsccli_sources} ${rsccli_headers})
target_link_libraries(rsccli
  common
  network
  rsclocal_com
  )

find_package(Qt5Widgets)
# find_package(Qt5Widgets CONFIG REQUIRED)

if( ${Qt5Widgets_FOUND} )
  message(STATUS "Qt5 found")
  message(STATUS "Configuring to build rscgui")

  file(GLOB rscgui_sources src/rscgui/*.cpp)
  file(GLOB rscgui_headers src/rscgui/*.hpp)

  add_executable(rscgui ${rscgui_sources} ${rscgui_headers})
  target_include_directories(rscgui PUBLIC ${Qt5Widgets_INCLUDE_DIRS})
  target_link_libraries(rscgui common)
  set_target_properties(rscgui PROPERTIES AUTOMOC TRUE)
  qt5_use_modules(rscgui Widgets)
else()
  message(WARNING "Qt5 not found.\n"
    "If you want to use rscgui, please install Qt5.")
endif()

if(INCLUDE_TEST)
  include(CTest)
  add_subdirectory(src/tests)
else()
  message(STATUS "Tests will not be included")
endif()

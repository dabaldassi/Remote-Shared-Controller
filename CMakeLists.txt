cmake_minimum_required( VERSION 3.14 )

project( remote-shared-controller )

option( INCLUDE_TEST OFF )
option( NOCURSOR OFF )
option( build_GUI ON )

set( core_exe ${CMAKE_PROJECT_NAME} )
set( cli_exe rsccli )
set( gui_exe rscgui )

find_package( Threads REQUIRED )

if( WIN32 )
  find_package( pthreads REQUIRED )
elseif( UNIX AND NOT APPLE )
  if( NOCURSOR )
    add_compile_options( -DNO_CURSOR )
  else()
    find_package( X11 COMPONENTS Xfixes )

    if( X11_FOUND AND X11_Xfixes_FOUND )
      message( STATUS "X11 is found, compiling with X11 features" )
    else()
      message( WARNING "X11 is not found, assuming there is no graphic server\n"
	"You must install X11 and X11 fixes to use X11 features" )
      add_compile_options( -DNO_CURSOR )
    endif()
  endif()
else()
  message( FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not handled" )
endif()

# -- Find Qt5 if GUI build is enabled
if( build_GUI )
  find_package( Qt5 COMPONENTS Widgets REQUIRED )
endif()

# -- Build main target
add_subdirectory( src )

# -- Build unit tests
if( INCLUDE_TEST )
  include( CTest )
  add_subdirectory( src/tests )
else()
  message( STATUS "Tests will not be included" )
endif()

add_subdirectory( install )

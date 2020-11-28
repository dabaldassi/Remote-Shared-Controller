
add_library( local_com )
add_library( rsc::localcom ALIAS local_com )

file( GLOB headers *.hpp )
file( GLOB sources *.cpp )

target_sources( local_com
  PRIVATE ${sources}
  PUBLIC  ${headers}
  )

target_include_directories( local_com PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

if( WIN32 )
  target_sources( local_com
    PRIVATE windows/fifo.cpp
    PUBLIC  windows/fifo.hpp
    )
elseif( UNIX AND NOT APPLE )
  target_sources( local_com
    PRIVATE linux/fifo.cpp
    PUBLIC  linux/fifo.hpp
    )
endif()

set_target_properties( local_com PROPERTIES CXX_STANDARD 14 )


add_library( common )
add_library( rsc::common ALIAS common )

file( GLOB_RECURSE sources *.cpp )
file( GLOB_RECURSE headers *.hpp )

target_sources( common
  PRIVATE ${sources}
  PUBLIC  ${headers}
  )
target_link_libraries( common
  rsc::controller
  rsc::localcom
  rsc::network
  )

target_include_directories( common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
set_target_properties( common PROPERTIES CXX_STANDARD 14 )

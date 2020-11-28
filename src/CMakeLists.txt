
# -- Build libraries
add_subdirectory( controller )
add_subdirectory( network )
add_subdirectory( rsclocal_com )
add_subdirectory( common )

# -- Build main executables

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

# -- Core
file( GLOB_RECURSE core_sources core/*.cpp )
file( GLOB_RECURSE core_headers core/*.hpp )

add_executable( ${core_exe}
  ${core_sources}
  ${core_headers}
  )

target_link_libraries( ${core_exe} PUBLIC rsc::common )
target_include_directories( ${core_exe} PUBLIC core )
set_target_properties( ${core_exe} PROPERTIES CXX_STANDARD 14 )

# -- CLI
file( GLOB_RECURSE rsccli_sources rsccli/*.cpp )
file( GLOB_RECURSE rsccli_headers rsccli/*.hpp )

add_executable( ${cli_exe}
  ${rsccli_sources}
  ${rsccli_headers}
  )

target_link_libraries( ${cli_exe} PUBLIC rsc::common )
target_include_directories( ${cli_exe} PUBLIC rsccli )
set_target_properties( ${cli_exe} PROPERTIES CXX_STANDARD 14 )

# -- GUI
if( build_GUI )
  message( STATUS "Configuring rscgui build" )

  file( GLOB rscgui_sources rscgui/*.cpp )
  file( GLOB rscgui_headers rscgui/*.hpp )

  if( WIN32 )
    set( PLATFORM_ARG WIN32 )
  endif()
  
  add_executable( ${gui_exe} ${PLATFORM_ARG}
    ${rscgui_sources}
    ${rscgui_headers}
    ${CMAKE_SOURCE_DIR}/resource/icon.rc
    ${CMAKE_SOURCE_DIR}/resource/res.qrc
    )

  target_include_directories( ${gui_exe} PUBLIC
    ${Qt5Widgets_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/rscgui
    )
  
  set_target_properties( ${gui_exe} PROPERTIES AUTOMOC TRUE )
  set_target_properties( ${gui_exe} PROPERTIES AUTORCC TRUE )
  set_target_properties( ${gui_exe} PROPERTIES CXX_STANDARD 14 )
  
  qt5_use_modules( ${gui_exe} Widgets )
  target_link_libraries( ${gui_exe} rsc::common )
endif()

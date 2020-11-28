
add_library( controller )
add_library( rsc::controller ALIAS controller )

target_sources( controller PRIVATE translate.c )

if( WIN32 )
  target_sources( controller PRIVATE
    windows/controller_win.cpp
    windows/cursor_win.cpp
    )
  set_target_properties( controller PROPERTIES CXX_STANDARD 14 )
elseif( UNIX AND NOT APPLE )
  target_sources( controller PRIVATE linux/controller_linux.c )
  if( NOT NOCURSOR )
    target_sources( controller PRIVATE linux/cursor_linux.c )
    target_link_libraries( controller PUBLIC
      ${X11_LIBRAIRIES}
      ${X11_X11_LIB}
      ${X11_Xfixes_LIB}
      )
    set_target_properties( controller PROPERTIES C_STANDARD 99 )
  endif()
endif()

target_include_directories( controller PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

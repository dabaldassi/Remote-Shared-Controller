
add_library( network )
add_library( rsc::network ALIAS network )

file( GLOB network_sources *.c )

target_sources( network PRIVATE ${network_sources} )

if( WIN32 )
  target_sources( network PRIVATE windows/scnp_socket.c )
  target_link_directories( network PUBLIC
    ${CMAKE_SOURCE_DIR}/external/npcap/Lib/x64
    )
  target_link_libraries( network PUBLIC
    ${PThreads4W_LIBRARY}
    wpcap.lib
    Packet.lib
    )
  target_include_directories( network PUBLIC
    ${PThreads4W_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/external/npcap/Include
    )
elseif( UNIX AND NOT APPLE )
  target_sources( network PRIVATE linux/scnp_socket.c )
endif()
    
target_link_libraries( network PUBLIC Threads::Threads )
target_include_directories( network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
set_target_properties( network PROPERTIES C_STANDARD 99 )

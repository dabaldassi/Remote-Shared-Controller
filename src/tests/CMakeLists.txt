message( STATUS "Tests will be included" )

# Compile the main only one time since a catch take a long long time to compile
add_library( catch OBJECT catch/main_catch.cpp )

# -- Event unit tests
add_executable( event_test event_test.cpp $<TARGET_OBJECTS:catch> )
target_link_libraries( event_test PUBLIC rsc::controller )

# -- Network unit tests
add_executable( network_test network_test.cpp $<TARGET_OBJECTS:catch> )
target_link_libraries( network_test PUBLIC rsc::network )

# -- Local communication unit tests
add_executable( localcom_test localcom_test.cpp $<TARGET_OBJECTS:catch> )
target_link_libraries( localcom_test PUBLIC rsc::localcom )

# -- Common unit tests
add_executable( common_test common_test.cpp $<TARGET_OBJECTS:catch> )
target_link_libraries( common_test PUBLIC rsc::common )

# -- CLI unit tests
add_executable( rsccli_test
  rsccli_test.cpp
  ../rsccli/command-cli.cpp
  ../rsccli/parser-cli.cpp
  $<TARGET_OBJECTS:catch>
  )

target_link_libraries( rsccli_test PUBLIC rsc::common  )
target_include_directories( rsccli_test PUBLIC ${CMAKE_SOURCE_DIR}/src/rsccli )

# -- Add unit tests exe to ctest
add_test( NAME event_test COMMAND event_test )
add_test( NAME network_test COMMAND network_test )
add_test( NAME localcom_test COMMAND localcom_test )
add_test( NAME common_test COMMAND common_test )
add_test( NAME rsccli_test COMMAND rsccli_test )

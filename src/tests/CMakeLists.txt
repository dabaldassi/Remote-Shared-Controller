
message(STATUS "Test will be included")

add_executable(event_test event_test.cpp catch/main_catch.cpp)
target_link_libraries(event_test controller pthread)

add_executable(localcom_test localcom_test.cpp catch/main_catch.cpp)
target_link_libraries(localcom_test rsclocal_com)

add_executable(common_test common_test.cpp catch/main_catch.cpp)
target_link_libraries(common_test common)

add_executable(rsccli_test
  rsccli_test.cpp
  catch/main_catch.cpp
  ../rsccli/command-cli.cpp
  ../rsccli/parser-cli.cpp
  )
target_link_libraries(rsccli_test
  common
  network
  rsclocal_com
  )

add_test(NAME event_test COMMAND event_test)
add_test(NAME localcom_test COMMAND localcom_test)
add_test(NAME common_test COMMAND common_test)
add_test(NAME rsccli_test COMMAND rsccli_test)

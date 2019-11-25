
message(STATUS "Test will be included")

add_executable(event_test event_test.cpp catch/main_catch.cpp)
target_link_libraries(event_test event_interface pthread)

add_test(NAME event_test
  COMMAND event_test)

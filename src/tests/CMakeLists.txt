
message(STATUS "Test will be included")

include(CTest)

add_executable(event_test event_test.cpp)
target_link_libraries(event_test event_interface)

add_test(NAME event_test
  COMMAND event_test)

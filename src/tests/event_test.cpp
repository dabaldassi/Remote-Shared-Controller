#include "catch/catch.hpp"
#include <string.h>
#include <iostream>
#include <chrono>
#include <thread>

#include <event_interface.h>

TEST_CASE("init/exit") {
  REQUIRE_FALSE(init_controller());
  REQUIRE(init_controller());

  exit_controller();

  REQUIRE_FALSE(init_controller());

  exit_controller();

}

TEST_CASE("put keys") {  
  using namespace std::chrono_literals;
  
  REQUIRE_FALSE(init_controller());

  std::this_thread::sleep_for(100ms); // init takes some time (needed for this test)
  
  write_key(KEY_A);
  write_key(KEY_ENTER);
  char a = getchar();
  getchar();

  REQUIRE(a == 'q');
  
  write_key(KEY_H);
  write_key(KEY_E);
  write_key(KEY_L);
  write_key(KEY_L);
  write_key(KEY_O);
  write_key(KEY_ENTER);

  char hello[6];
  fgets(hello,6,stdin);
  hello[6] = 0;
  
  REQUIRE_FALSE(strcmp(hello,"hello"));
  
  exit_controller();
}

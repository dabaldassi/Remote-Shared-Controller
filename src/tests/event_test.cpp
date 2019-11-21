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

  std::this_thread::sleep_for(300ms); // init takes some time (needed for this test)
  
  write_key(KEY_SPACE);
  write_key(KEY_ENTER);
  char a = getchar();
  getchar();

  REQUIRE(a == ' ');
  
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

TEST_CASE("get keys") {
  using namespace std::chrono_literals;

  unsigned short code;
  int            val;

  REQUIRE_FALSE(init_controller());

  std::this_thread::sleep_for(300ms); // init takes some time (needed for this test)
  
  write_key(KEY_A);
  write_key(KEY_Q);

  bool ret = get_key(&code, &val);
  REQUIRE((ret && code == KEY_A && val == EV_PRESSED));

  ret = get_key(&code, &val);
  
  REQUIRE(ret);
  REQUIRE(code == KEY_A);
  REQUIRE(val == EV_RELEASED);

  ret = get_key(&code, &val);
  REQUIRE((ret && code == KEY_Q && val == EV_PRESSED));
  
  ret = get_key(&code, &val);
  REQUIRE((ret && code == KEY_Q && val == EV_RELEASED));

  /* Remove key wrote ont the terminal */
  write_key(KEY_BACKSPACE);
  write_key(KEY_BACKSPACE);

  write_key_ev(KEY_LEFTSHIFT, EV_PRESSED);
  write_key(KEY_A);
  write_key_ev(KEY_LEFTSHIFT, EV_RELEASED);

  write_key(KEY_BACKSPACE);
  
  exit_controller();
}

TEST_CASE("test")
{
  using namespace std::chrono_literals;

  unsigned short code;
  int            val;

  REQUIRE_FALSE(init_controller());

  std::this_thread::sleep_for(300ms); // init takes some time (needed for this test)

  bool quit = false;
  while(!quit) {
    bool ret = get_key(&code,&val);

    if(ret) {
      std::cout << code << "\n";
    }

    quit = code == KEY_TAB;
  }
  
  exit_controller();
}

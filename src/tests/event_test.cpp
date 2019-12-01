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

TEST_CASE("Move Mouse")
{
  using namespace std::chrono_literals;
  
  REQUIRE_FALSE(init_controller());

  std::this_thread::sleep_for(300ms); // init takes some time (needed for this test)

  int x = 5, y = 5, i = 100;

  while(i--) {
    mouse_move(x,y);
    std::this_thread::sleep_for(10ms);
  }

  exit_controller();
}

TEST_CASE("controllerevent")
{
  using namespace std::chrono_literals;

  ControllerEvent ce;
  bool grab = false;
  
  REQUIRE_FALSE(init_controller());

  std::this_thread::sleep_for(300ms); // init takes some time (needed for this test)

  int quit = 0;
  while(!quit) {
    int ret = poll_controller(&ce);

    if(!ret) {
      
      if(grab) {
	write_controller(&ce);
      }
      
      if(ce.controller_type == EV_KEY && ce.value == KEY_RELEASED && ce.code == KEY_SPACE) {
	grab = !grab;
	grab_controller(grab);
      }
      
      quit = ce.code == KEY_TAB;
      std::cout << ce.code << " " << ce.value << "\n";
    }
  }

  exit_controller();
}

#ifdef GRAB_TEST

TEST_CASE("test")
{
  using namespace std::chrono_literals;

  unsigned short code;
  int            val;
  bool           grab = true;

  REQUIRE_FALSE(init_controller());

  std::this_thread::sleep_for(300ms); // init takes some time (needed for this test)

  grab_controller(grab);
  bool quit = false;

  while(!quit) {
    bool ret = get_key(&code,&val);

    if(ret) {
      std::cout << code << "\n";

      if(val == 0 && code == KEY_SPACE) {
	grab = !grab;
	grab_controller(grab);
      }
    }

    quit = code == KEY_TAB;
    
  }
  
  exit_controller();
}

#endif

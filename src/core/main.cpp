#include <iostream>
#include <cstring>
#include <scnp.h>
#include <rsc.hpp>
#include <util.hpp>

#include <unistd.h>

int main(int argc, char *argv[])
{
  rscutil::register_pid();
  
  RSC rsc;
  int if_index = 1;

  if(argc == 2) {
    if_index = atoi(argv[1]);
  }
  
  rsc.init(if_index);
  sleep(1);
  
  do {
    rsc.run();
    if(rsc.is_paused()) rsc.wait_for_wakeup();
    
  }while(rsc.is_running());
  
  rsc.exit();
  
  return 0;
}

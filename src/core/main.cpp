#include <iostream>
#include <cstring>
#include <rsc.hpp>
#include <scnp.h>
#include <util.hpp>


int main(int argc, char *argv[])
{
  rscutil::register_pid();
  
  RSC rsc;
  int if_index = 8;

  if(argc == 2) {
    if_index = atoi(argv[1]);
  }
  
  rsc.init(if_index);
  
  do {
    rsc.run();
    if(rsc.is_paused()) rsc.wait_for_wakeup();
    
  }while(rsc.is_running());
  
  rsc.exit();
  
  return 0;
}

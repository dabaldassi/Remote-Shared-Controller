#include <iostream>
#include <cstring>
#include <scnp.h>
#include <rsc.hpp>
#include <util.hpp>

int main(int argc, char *argv[])
{
  rscutil::register_pid();
  
  RSC rsc;

  if(argc == 2) rsc.set_interface(atoi(argv[1]));
  
  rsc.init();

  do {
    rsc.run();
    if(rsc.is_paused()) rsc.wait_for_wakeup();
    
  }while(rsc.is_running());
  
  rsc.exit();
  
  return 0;
}

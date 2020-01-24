#include <iostream>
#include <cstring>
#include <scnp.h>
#include <rsc.hpp>
#include <util.hpp>

int main(int argc, char *argv[])
{
  rscutil::register_pid();
  
  RSC rsc;

  if(argc == 2) {
    int if_index = atoi(argv[1]);
    
    if(rsc.set_interface(if_index)) {
      throw std::invalid_argument("This is not a valid interface");
    }
  }
  
  rsc.init();

  do {
    rsc.run();
    if(rsc.is_paused()) rsc.wait_for_wakeup();
    
  }while(rsc.is_running());
  
  rsc.exit();
  
  return 0;
}

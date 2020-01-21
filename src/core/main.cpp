#include <iostream>
#include <cstring>
#include <scnp.h>
#include <rscp.hpp>
#include <util.hpp>

int main(int argc, char *argv[])
{
  rscutil::register_pid();
  
  RSCP rscp;

  if(argc == 2) rscp.set_interface(atoi(argv[1]));
  
  rscp.init();

  do {
    rscp.run();
    if(rscp.is_paused()) rscp.wait_for_wakeup();
    
  }while(rscp.is_running());
  
  rscp.exit();
  
  return 0;
}

#include <iostream>
#include <cstring>
#include <scnp.h>
#include <rscp.hpp>

int main(int argc, char *argv[])
{
  RSCP rscp;

  if(argc == 2) rscp.set_interface(atoi(argv[1]));
  
  rscp.init();
  rscp.run();
  rscp.exit();
  
  return 0;
}

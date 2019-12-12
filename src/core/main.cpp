#include <iostream>
#include <cstring>
#include <event_interface.h>
#include <scnp.h>
#include <rscp.hpp>

int main(int argc, char *argv[])
{
  const uint8_t dest_addr[] = { 0x84, 0x16, 0xf9, 0x3a, 0x3a, 0xad }; // brutalpc
  // const uint8_t dest_addr[] = { 0x80,0xc5,0xf2,0x32,0x43,0xe9 }; // vivodebian
  // const uint8_t dest_addr[] = { 0x08, 0x00, 0x27, 0x37, 0x69, 0xa6 }; // AlexVM
  // const uint8_t dest_addr[] = { 0x00, 0x26, 0x18, 0xf0, 0xdd, 0xb5 }; // ikone06
  // const uint8_t dest_addr[] = { 0x08, 0x00, 0x27, 0xd8, 0x89, 0x21 }; // JulesVM
  // const uint8_t dest_addr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // local

  RSCP rscp;
  PC   remote;

  remote.local = false;
  memcpy(remote.adress, dest_addr, 6);
  remote.id = 12;
  remote.name = "Remote PC 1";
 
  rscp.get_config().add(remote);

  if(argc == 2) rscp.set_interface(atoi(argv[1]));
  
  rscp.init();
  rscp.run();
  rscp.exit();
  
  return 0;
}

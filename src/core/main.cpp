#include <iostream>
#include <cstring>
#include <scnp.h>
#include <rscp.hpp>

int main(int argc, char *argv[])
{
  // const uint8_t dest_addr[] = { 0x84, 0x16, 0xf9, 0x3a, 0x3a, 0xad }; // brutalpc
  // const uint8_t dest_addr[] = { 0x80,0xc5,0xf2,0x32,0x43,0xe9 }; // vivodebian
  // const uint8_t dest_addr[] = { 0x08, 0x00, 0x27, 0x37, 0x69, 0xa6 }; // AlexVM
  // const uint8_t dest_addr[] = { 0x00, 0x26, 0x18, 0xf0, 0xdd, 0xb5 }; // ikone06
  const uint8_t dest_addr[] = { 0x00, 0x26, 0x18, 0xf0, 0xdc, 0xe4 }; // ikone13
  // const uint8_t dest_addr[] = { 0x08, 0x00, 0x27, 0xd8, 0x89, 0x21 }; // JulesVM
  // const uint8_t dest_addr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // local

  RSCP rscp;
  PC   remote,remote2;

  remote.local /* = remote2.local*/ = false;
  memcpy(remote.address, dest_addr, PC::LEN_ADDR);
  // memcpy(remote2.adress, dest_addr2, PC::LEN_ADDR);
  remote.id = 12;
  // remote2.id = 13;
  remote.name = "Remote PC 1";
  remote.resolution.w = 1920;
  remote.resolution.h = 1080;
  // remote2.name = "Remote PC 2";
 
  rscp.get_config().add(remote);
  // rscp.get_config().add(remote2);

  if(argc == 2) rscp.set_interface(atoi(argv[1]));
  
  rscp.init();
  rscp.run();
  rscp.exit();
  
  return 0;
}

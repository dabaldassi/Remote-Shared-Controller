#include <iostream>

#include <event_interface.h>
#include <scnp.h>
#include <rscp.hpp>

int main(int argc, char *argv[])
{
  // struct scnp_socket sock;
  // const uint8_t dest_addr[] = { 0,0,0,0,0,0 };

  // int err = scnp_create_socket(&sock, 1);

  // if(err) {
  //   perror("ERROR");
  //   std::exit(1);
  // }

  // struct scnp_key key_req;
  // key_req.type = EV_KEY;
  // key_req.code = KEY_A;
  // key_req.flags = KEY_PRESSED;

  // scnp_send(&sock, dest_addr, (struct scnp_packet*)&key_req, sizeof(key_req));

  // scnp_close_socket(&sock);

  RSCP rscp;
  
  rscp.init();
  rscp.run();
  rscp.exit();
  
  return 0;
}

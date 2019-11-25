#include <iostream>

#include <event_interface.h>
#include <network.h>

int main(int argc, char *argv[])
{
  constexpr int LOOPBACK = 1;
  constexpr int LAMIENNE = 4;

  int sock = create_scnp_socket(LAMIENNE);

  int err = init_controller();

  if(err) std::exit(1);
  
  if(argc == 1) {
    const uint8_t L_ADR[] = {0x08,0x00,0x27,0x37,0x69,0xa6};

    scnp_init(sock, LAMIENNE);
    scnp_input(sock, LAMIENNE, L_ADR, 30, 1);
  }
  else if(argc == 2) {
    struct scnp_req request;

    while(1) {
      if(recv_scnp_request(sock, LAMIENNE, &request)) {
	if(request.flags == 0x80) std::cout << "INIT" << "\n";
	else {
	  int code = request.input;
	  int val = (request.flags >> 5) & 0x01;

	  write_key_ev(code, val);
	}
      }      
    } 
  }
  
  return 0;
}

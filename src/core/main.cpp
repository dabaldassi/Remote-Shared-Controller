#include <iostream>

#include <event_interface.h>
#include <network.h>

int main(int argc, char *argv[])
{
  constexpr int LOOPBACK = 1;
  constexpr int LAMIENNE = 6;

  struct scnp_socket sock;
  create_scnp_socket(&sock, LAMIENNE);

  int err = init_controller();

  if(err) std::exit(1);
  
  if(argc == 1) {
    const uint8_t L_ADR[] = {0x08,0x00,0x27,0x37,0x69,0xa6};

    scnp_send_key(&sock, L_ADR, EV_KEY, KEY_PRESSED, KEY_Q);
    scnp_send_key(&sock, L_ADR, EV_KEY, KEY_RELEASED, KEY_Q);
  }
  else if(argc == 2) {
    struct scnp_req request;

    while(1) {
      recv_scnp_request(&sock, &request);
      printf("%u %u %u\n", request.type, request.code, request.value);

      switch(request.type) {
      case EV_KEY:
	write_key_ev(request.code, request.value);
	break;
      case EV_REL:
	int x = 0, y = 0;
	
	if(request.code == REL_X) x = request.value;
	else                      y = request.value;

	mouse_move(x, y);
	break;
      }
    } 
  }
  
  return 0;
}

#include <thread>
#include <functional>
#include <iostream>

#include <rscp.hpp>
#include <event_interface.h>
#include <convkey.hpp>
#include <chrono>

void error(const char * s)
{
  perror(s);
  std::exit(EXIT_FAILURE);
}

RSCP::RSCP(): _state(State::HERE)
{

}

int RSCP::init()
{
  int err = scnp_create_socket(&_sock, IF);

  if(err) error("Can't create socket");

  err = init_controller();
  if(err) error("Can't init controller");

  _run = true;
  _transition = false;
  
  return 0;
}

void RSCP::exit()
{
  exit_controller();
  scnp_close_socket(&_sock);
}

void RSCP::_transit()
{
  if (_state == State::HERE) {
      grab_controller(true);
      _state = State::AWAY;
  }
  else {
    grab_controller(false);
    _state = State::HERE;
  }

  _transition = false;
}

void RSCP::_receive()
{
  struct scnp_packet   packet;
  ControllerEvent    * ev = nullptr;
  
  while(_run) {
    scnp_recv(&_sock, &packet);

    switch(packet.type) {
    case EV_KEY: ev = ConvKey<ControllerEvent,KEY>::get(packet); break;
    case EV_ABS: // [BUG] Can't move mouse in ABS for now
    case EV_REL: ev = ConvKey<ControllerEvent,MOUSE>::get(packet);
      break;
    default:
      break;
    }
    
    write_controller(ev);
  }
}


void RSCP::_send(const ControllerEvent &ev)
{
  const uint8_t dest_addr[] = { 0, 0, 0, 0, 0, 0 };

  switch(ev.controller_type) {
  case MOUSE:
    scnp_send(&_sock,
	      dest_addr,
	      ConvKey<struct scnp_packet, MOUSE>::get(ev),
	      ConvKey<struct scnp_packet, MOUSE>::SIZE);
    break;
  case KEY:
    scnp_send(&_sock,
	      dest_addr,
	      ConvKey<struct scnp_packet, KEY>::get(ev),
	      ConvKey<struct scnp_packet, KEY>::SIZE);
    break;
  default:
    break;
  }
}

void RSCP::run()
{
  // always read in a thread
  ControllerEvent c;
  
  std::thread(std::bind(&RSCP::_receive, this)).detach();
  
  while(_run) {
    int ret = poll_controller(&c);
    if(ret) continue;

    if(c.code == KEY_TAB && c.value == KEY_RELEASED) _transition = true;
    
    if(c.code == KEY_BACKSPACE) {
      _run = false;
      continue;
    }
    
    switch(_state) {
    case State::HERE:
      if(_transition) _transit();
      break;
    case State::AWAY:
      if(_transition) _transit();
      else            _send(c);
      break;
    }
  }
}

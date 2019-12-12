#include <thread>
#include <functional>
#include <iostream>

#include <rscp.hpp>
#include <event_interface.h>
#include <convkey.hpp>

void error(const char * s)
{
  perror(s);
  std::exit(EXIT_FAILURE);
}

RSCP::RSCP(): _if(DEFAULT_IF), _state(State::HERE)
{
  auto sh_ptr = ComboShortcut::make_ptr();

  sh_ptr->add_shortcut(KEY_LEFTCTRL, KEY_PRESSED);
  sh_ptr->add_shortcut(KEY_R, KEY_PRESSED);
  sh_ptr->add_shortcut(KEY_RIGHT, KEY_PRESSED);  
  sh_ptr->release_for_all();
  
  _swap = std::move(sh_ptr);

  _quit_shortcut.add_shortcut(KEY_ESC, KEY_PRESSED, 200);
  _quit_shortcut.add_shortcut(KEY_ESC, KEY_RELEASED, 200);
  _quit_shortcut.add_shortcut(KEY_ESC, KEY_PRESSED, 200);
  _quit_shortcut.add_shortcut(KEY_ESC, KEY_RELEASED, 200);
  _quit_shortcut.add_shortcut(KEY_ESC, KEY_PRESSED, 200);
  _quit_shortcut.add_shortcut(KEY_ESC, KEY_RELEASED, 200);

  PC local_pc;

  local_pc.local = true;
  local_pc.id = 0;
  local_pc.name = "localhost";
  
  _pc_list.add(local_pc);
}

int RSCP::init()
{
  int err = scnp_create_socket(&_sock, _if);

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
  if(_swap->get_way() == Combo::Way::LEFT) _pc_list.previous_pc();
  else                                     _pc_list.next_pc();

  std::cout << _pc_list.get_current().name << "\n";
  grab_controller(!_pc_list.get_current().local);
  _state = (_pc_list.get_current().local)? State::HERE : State::AWAY;
  _transition = false;
}

void RSCP::_receive()
{
  struct scnp_packet   packet;
  ControllerEvent    * ev = nullptr;
  
  while(_run) {
    scnp_recv(&_sock, &packet);

    switch(packet.type) {
    case EV_KEY: ev = ConvKey<ControllerEvent,KEY>::get(packet);   break;
    case EV_ABS: // [BUG] Can't move mouse in ABS for now
    case EV_REL: ev = ConvKey<ControllerEvent,MOUSE>::get(packet); break;
    default:     ev = NULL;                                        break;
    }
    
    if(ev) write_controller(ev);
  }
}


void RSCP::_send(const ControllerEvent &ev)
{  
  switch(ev.controller_type) {
  case MOUSE:
    scnp_send(&_sock,
	      _pc_list.get_current().adress,
	      ConvKey<struct scnp_packet, MOUSE>::get(ev),
	      ConvKey<struct scnp_packet, MOUSE>::SIZE);
    break;
  case KEY:
    scnp_send(&_sock,
	      _pc_list.get_current().adress,
	      ConvKey<struct scnp_packet, KEY>::get(ev),
	      ConvKey<struct scnp_packet, KEY>::SIZE);
    break;
  default:
    break;
  }
}

void RSCP::run()
{
  ControllerEvent c;

  // always read in a thread
  std::thread(std::bind(&RSCP::_receive, this)).detach();
  
  while(_run) {
    int ret = poll_controller(&c);
    if(!ret) continue;

    _run = !_quit_shortcut.update(c.code, c.value);
    if(!_run) continue;
    
    _transition = _swap->update(c.code, c.value);
    
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

#include <thread>
#include <future>
#include <functional>
#include <iostream>
#include <csignal>

#include <rscp.hpp>

#include <controller.h>
#include <cursor.h>

#include <convkey.hpp>

void error(const char * s)
{
  perror(s);
  std::exit(EXIT_FAILURE);
}

RSCP::RSCP(): _if(DEFAULT_IF), _state(State::HERE)
{
  _cursor = open_cursor_info();
  
  auto sh_ptr = ComboShortcut::make_ptr();

  sh_ptr->add_shortcut(KEY_LEFTCTRL, KEY_PRESSED);
  sh_ptr->add_shortcut(KEY_R, KEY_PRESSED);
  sh_ptr->add_shortcut(KEY_RIGHT, KEY_PRESSED);  
  sh_ptr->release_for_all();
  sh_ptr->set_action([this](Combo * combo) {
		       _transit(combo->get_way());
		     });
  
  _shortcut.push_back(std::move(sh_ptr));
  
  auto quit_shortcut = ComboShortcut::make_ptr();

  for(int i = 0; i < 3; i++) {
    quit_shortcut->add_shortcut(KEY_ESC, KEY_PRESSED, 200);
    quit_shortcut->add_shortcut(KEY_ESC, KEY_RELEASED, 200);
  }
  
  quit_shortcut->set_action([this](Combo*) { _run = false; });
  
  _shortcut.push_back(std::move(quit_shortcut));
    
  PC local_pc;

  local_pc.local = true;
  local_pc.id = 0;
  local_pc.name = "localhost";
  local_pc.resolution.w = 1920;
  local_pc.resolution.h = 1080;
  
  _pc_list.add(local_pc);

  if(_cursor) {
    _shortcut.push_back(ComboMouse::make_ptr(local_pc.resolution.w,
					     local_pc.resolution.h,
					     _cursor));
    _shortcut.back()->set_action([this](Combo* combo) {
				   int s = (combo->get_way() == Combo::Way::LEFT)?1:-1;
				   mouse_move(s * 5, 0);
				   _transit(combo->get_way());
				 });
  }
  
}

RSCP::~RSCP()
{
  if(_cursor) close_cursor_info(_cursor);
}

int RSCP::init()
{
  int err = scnp_create_socket(&_sock, _if);

  if(err) error("Can't create socket");
  
  err = init_controller();
  if(err) error("Can't init controller");

  _run = true;
  
  return 0;
}

void RSCP::exit()
{
  exit_controller();
  scnp_close_socket(&_sock);
}

void RSCP::_transit(Combo::Way way)
{  
  if(way == Combo::Way::LEFT) _pc_list.previous_pc();
  else                        _pc_list.next_pc();

  grab_controller(!_pc_list.get_current().local);
 
  _state = (_pc_list.get_current().local)? State::HERE : State::AWAY;

  if(_state == State::AWAY) hide_cursor(_cursor);
  else                      show_cursor(_cursor);
}

void RSCP::_receive()
{
  constexpr int OUT_LEFT  = 5;
  constexpr int OUT_RIGHT = 6;
  
  struct scnp_packet   packet;
  ControllerEvent    * ev = nullptr;
  uint8_t              addr_src[PC::LEN_ADDR];
  const PC&            local_pc = _pc_list.get_local();
  ComboMouse           mouse(local_pc.resolution.w, local_pc.resolution.h,_cursor);
  
  mouse.set_action([&](Combo* combo) {
		     auto way = combo->get_way();
		     int sens = (way == Combo::Way::LEFT)?1:-1;
		     mouse_move(sens * 5, 0);
		     
		     struct scnp_packet pack;
		     pack.type = (Combo::Way::LEFT == way) ? OUT_LEFT : OUT_RIGHT;
		     scnp_send(&_sock,addr_src,&pack,sizeof(pack));
		   });

  while(_run) {
    scnp_recv_from(&_sock, &packet, addr_src);
    
    switch(packet.type) {
    case EV_KEY:    ev = ConvKey<ControllerEvent,KEY>::get(packet);   break;
    case EV_ABS:    // [BUG] Can't move mouse in ABS for now
    case EV_REL:    ev = ConvKey<ControllerEvent,MOUSE>::get(packet); break;
    case OUT_LEFT:  _transit(Combo::Way::LEFT); ev = nullptr;         break;
    case OUT_RIGHT: _transit(Combo::Way::RIGHT); ev = nullptr;        break;
    default:        ev = nullptr;                                     break;
    }
    
    if(ev) {
      write_controller(ev);
      if(ev->controller_type == MOUSE) mouse.update(ev->code, ev->value);
    }
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
  std::thread(std::bind(&RSCP::_receive, this)).detach(); // [BUG] : Memory leak
  
  while(_run) {
    int ret = poll_controller(&c);
    if(!ret) continue;

    for(auto&& s : _shortcut) s->update(c.code, c.value);
    
    switch(_state) {
    case State::HERE:           break;
    case State::AWAY: _send(c); break;
    }
  }
}

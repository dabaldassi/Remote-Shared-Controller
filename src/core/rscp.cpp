#include <functional>
#include <string>
#include <cstring>
#include <algorithm>

#include <controller.h> // Must be before convkey.hpp
#include <rscp.hpp> // Also before convkey.hpp

#include <convkey.hpp>
#include <config.hpp>

void error(const char * s)
{
  perror(s);
  std::exit(EXIT_FAILURE);
}

template<typename Mutex, typename Lambda>
void RSCP::_th_safe_op(Mutex &m, Lambda &&l)
{
  std::unique_lock<Mutex> lock(m);
  l();
}

RSCP::RSCP(): _if(DEFAULT_IF), _next_pc_id{0}, _state(State::HERE)
{
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
    
  PC local_pc { _next_pc_id++, true, "localhost", {0}, {1920,1080}, {0,0}};
  
  _pc_list.add(local_pc);

#ifndef NO_CURSOR
  _cursor = open_cursor_info();

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
#endif
}

RSCP::~RSCP()
{
#ifndef NO_CURSOR
  if(_cursor) close_cursor_info(_cursor);
#endif
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

#ifndef NO_CURSOR
  if(_state == State::AWAY) hide_cursor(_cursor);
  else                      show_cursor(_cursor);
#endif
}

void RSCP::add_pc(const uint8_t *addr, const std::string& hostname)
{
  bool exist = _all_pc_list.exist([&addr](const PC& pc) -> bool {
				    return !memcmp(addr, pc.address, PC::LEN_ADDR);
				  });
  if(!exist) {
    PC pc{ _next_pc_id++, false, hostname, {0}, { 0,0 }, { 0,0 }}; 

    memcpy(pc.address, addr, PC::LEN_ADDR);
    
    _th_safe_op(_all_pc_list_mutex, [this,&pc]() {_all_pc_list.add(pc); });
    _th_safe_op(_alive_mutex, [this, &pc]() { _alive[pc.id] = clock_t::now(); });
  }
  else {
    int id = _all_pc_list.get([&addr](const PC& pc) -> bool {
				    return !memcmp(addr, pc.address, PC::LEN_ADDR);
			      }).id;
    _th_safe_op(_alive_mutex, [&id,this]() { _alive[id] = clock_t::now(); } );
  }
}

void RSCP::_receive()
{
  constexpr int OUT_LEFT  = 0;
  constexpr int OUT_RIGHT = 1u << 7;
  
  struct scnp_packet   packet;
  ControllerEvent    * ev = nullptr;
  uint8_t              addr_src[PC::LEN_ADDR];

#ifndef NO_CURSOR
  const PC&            local_pc = _pc_list.get_local();
  ComboMouse           mouse(local_pc.resolution.w, local_pc.resolution.h,_cursor);
  
  mouse.set_action([&](Combo* combo) {
		     auto way = combo->get_way();
		     int sens = (way == Combo::Way::LEFT)?1:-1;
		     mouse_move(sens * 5, 0);
		     
		     struct scnp_out pkt;
		     pkt.type = SCNP_OUT;
		     pkt.flags = 0x00;
		     pkt.flags |= (Combo::Way::LEFT == way) ? OUT_LEFT : OUT_RIGHT;
		     scnp_send(&_sock,addr_src,
			       reinterpret_cast<struct scnp_packet *>(&pkt),
			       sizeof(pkt));
		   });
#endif

  std::map<uint8_t, std::function<ControllerEvent *(void)>> on_packet =
    {
     { EV_KEY, [&packet]() { return ConvKey<ControllerEvent,KEY>::get(packet); }},
     { EV_REL, [&packet]() { return ConvKey<ControllerEvent,MOUSE>::get(packet); }},
     { EV_ABS, []() { return nullptr; } },
     { SCNP_OUT, [&packet,this]() {
		   auto * pkt = reinterpret_cast<struct scnp_out*>(&packet);
		   if(!(pkt->flags & OUT_LEFT)) _transit(Combo::Way::LEFT);
		   else                         _transit(Combo::Way::RIGHT);
		   return nullptr;
		 }},
     { SCNP_MNGT, [this, &addr_src, &packet]() {
		    auto * pkt = reinterpret_cast<struct scnp_management*>(&packet);
		    add_pc(addr_src, pkt->hostname);
		    return nullptr; }},
    };

  while(_run) {
    scnp_recv_from(&_sock, &packet, addr_src);
    
    auto it = on_packet.find(packet.type);
    
    if(it != on_packet.end()) ev = it->second();
    else                      ev = nullptr;
    
    if(ev) {
      write_controller(ev);

#ifndef NO_CURSOR
      if(ev->controller_type == MOUSE) mouse.update(ev->code, ev->value);
#endif
    }
  }
}


void RSCP::_send(const ControllerEvent &ev)
{
  switch(ev.controller_type) {
  case MOUSE:
    scnp_send(&_sock,
	      _pc_list.get_current().address,
	      ConvKey<struct scnp_packet, MOUSE>::get(ev),
	      ConvKey<struct scnp_packet, MOUSE>::SIZE);
    break;
  case KEY:
    scnp_send(&_sock,
	      _pc_list.get_current().address,
	      ConvKey<struct scnp_packet, KEY>::get(ev),
	      ConvKey<struct scnp_packet, KEY>::SIZE);
    break;
  default:
    break;
  }
}

void RSCP::_local_cmd()
{
  using namespace rsclocalcom;
  Message     msg, ack(Message::ACK);
  bool        pause_request = false, stop_request = false;

  std::map<Message::Command, std::function<void(const Message&)>> on_msg = 
    {
      { Message::IF, [this,&ack](const Message& m) {
	  set_interface(std::stoi(m.get_arg(0)));
	  ack.add_arg(Message::OK);  }},
      { Message::GETLIST, [this, &ack](const Message& ) {
	  _pc_list.save(CURRENT_PC_LIST);
	  _all_pc_list.save(ALL_PC_LIST);
	  ack.add_arg(Message::OK); } },
      { Message::SETLIST, [this, &ack](const Message& ) {
	  _th_safe_op(_pc_list_mutex, [this]() {_pc_list.load(CURRENT_PC_LIST);});
	  _th_safe_op(_all_pc_list_mutex,[this](){_all_pc_list.load(ALL_PC_LIST);});
	  ack.add_arg(Message::OK);
	}},
      { Message::START, [&ack](const Message&) {
	  ack.add_arg(Message::STARTED);
	}},
      { Message::STOP, [&stop_request, &ack](const Message&) {
	  stop_request = true;
	  ack.add_arg(Message::OK);
	}},
      { Message::PAUSE, [&pause_request, &ack](const Message&) {
	  pause_request = true;
	  ack.add_arg(Message::OK);
	}},
    };

  while(_run) {
    ack.reset(Message::ACK);
   
    _com.read_from(RSCLocalCom::Contact::CLIENT, msg);
    on_msg[msg.get_cmd()](msg);
    _com.send_to(RSCLocalCom::Contact::CLIENT, ack);

    if(pause_request)     pause_requested();
    else if(stop_request) stop_requested();
  }

}

void RSCP::_keep_alive()
{
  decltype(_alive)::iterator it;
  
  while(_run) {

    do {
      it = std::find_if(_alive.begin(),
			_alive.end(),
			[](const auto& a) {
			  auto now = RSCP::clock_t::now();
			  std::chrono::duration<double> elapsed;
			  elapsed = now - a.second;
			  return elapsed.count() > ALIVE_TIMEOUT;
			});

      if(it != _alive.end()) {
	_th_safe_op(_pc_list_mutex, [&it,this]() {_pc_list.remove(it->first); });
	_th_safe_op(_all_pc_list_mutex, [&it, this]() { _all_pc_list.remove(it->first); });
	_th_safe_op(_alive_mutex, [&it, this]() { _alive.erase(it); } );
      }
      
    } while(it != _alive.end());

    std::this_thread::sleep_for(std::chrono::seconds(int{ALIVE_TIMEOUT}));
  }
}

void RSCP::_send()
{
  ControllerEvent c;
  
  while(_run) {
    int ret = poll_controller(&c);
    if(!ret) continue;
    if(ret & 0x02) {
    }
    if(ret & 0x01) {

      for(auto&& s : _shortcut) s->update(c.code, c.value);
    
      switch(_state) {
      case State::HERE:           break;
      case State::AWAY: _send(c); break;
      }
    }
  }

  stop_requested();
}

void RSCP::run()
{
  scnp_start_session(_if);
  
  _pause = false;
  _run = true;
  
  _threads.push_back(std::thread(std::bind(&RSCP::_receive, this)));
  _threads.push_back(std::thread([this]() { _send(); }));
  _threads.push_back(std::thread(std::bind(&RSCP::_keep_alive, this)));
  _threads.push_back(std::thread(&RSCP::_local_cmd, this));

  for(auto&& th : _threads) th.join();

  _threads.clear();
  scnp_stop_session(_if);
}

void RSCP::pause_requested()
{
  _pause = true;
  for(auto&& th : _threads) pthread_cancel(th.native_handle());  
}

void RSCP::stop_requested()
{
  _run = false;
  for(auto&& th : _threads) pthread_cancel(th.native_handle());
}

void RSCP::set_interface(int index)
{
  scnp_stop_session(_if);
  _if = index;
  _sock.if_index = index;
  scnp_start_session(_if);
}

void RSCP::wait_for_wakeup()
{
  using namespace rsclocalcom;
  Message msg, ack(Message::ACK);

  while(_pause) {
    _com.read_from(RSCLocalCom::Contact::CLIENT, msg);
    ack.reset(Message::ACK);

    switch(msg.get_cmd()) {
    case Message::START:
      _pause = false;
      ack.add_arg(Message::OK);
      break;
    case Message::STOP:
      _pause = false;
      _run = false;
      ack.add_arg(Message::OK);
      break;
    default:
      ack.add_arg(Message::PAUSED);
      break;
    }

    _com.send_to(RSCLocalCom::Contact::CLIENT, ack);
  }
}

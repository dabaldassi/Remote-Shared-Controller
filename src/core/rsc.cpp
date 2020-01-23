#include <functional>
#include <string>
#include <cstring>
#include <algorithm>

#include <controller.h> // Must be before convkey.hpp
#include <rsc.hpp> // Also before convkey.hpp

#include <convkey.hpp>
#include <config.hpp>

void error(const char * s)
{
  perror(s);
  std::exit(EXIT_FAILURE);
}

template<typename Mutex, typename Lambda>
void RSC::_th_safe_op(Mutex &m, Lambda &&l)
{
  std::unique_lock<Mutex> lock(m);
  l();
}

RSC::RSC(): _if(DEFAULT_IF), _next_pc_id{0},
	      _com(rsclocalcom::RSCLocalCom::Contact::CORE),
	      _state(State::HERE)
{
  using namespace rscutil;
  auto sh_ptr = ComboShortcut::make_ptr("right", "Move to the next computer on the right");

  sh_ptr->add_shortcut(KEY_LEFTCTRL, KEY_PRESSED);
  sh_ptr->add_shortcut(KEY_R, KEY_PRESSED);
  sh_ptr->add_shortcut(KEY_RIGHT, KEY_PRESSED);  
  sh_ptr->release_for_all();
  sh_ptr->set_action([this](Combo * combo) {
      _transit(combo->get_way());
    });
  
  _shortcut.push_back(std::move(sh_ptr));
  
  auto quit_shortcut = ComboShortcut::make_ptr("quit", "Quit the service");

  for(int i = 0; i < 3; i++) {
    quit_shortcut->add_shortcut(KEY_ESC, KEY_PRESSED, 200);
    quit_shortcut->add_shortcut(KEY_ESC, KEY_RELEASED, 200);
  }
  
  quit_shortcut->set_action([this](Combo*) { _run = false; });
  
  _shortcut.push_back(std::move(quit_shortcut));
    
  PC local_pc { _next_pc_id++, true, "localhost", {0}, {0,0}, {0,0}};

#ifndef NO_CURSOR
  _cursor = open_cursor_info();

  local_pc.resolution.w = _cursor->screen_size.width;
  local_pc.resolution.h = _cursor->screen_size.height;    
  
  if(_cursor) {
    _shortcut.push_back(ComboMouse::make_ptr(local_pc.resolution.w,
					     local_pc.resolution.h,
					     _cursor));
    _shortcut.back()->set_action([this](Combo* combo) {
	float height = (float) _cursor->pos_y /
	  _cursor->screen_size.height;
	_transit(combo->get_way(), height);
      });
    
  }
#endif

  _pc_list.add(local_pc);
}

RSC::~RSC()
{
#ifndef NO_CURSOR
  if(_cursor) close_cursor_info(_cursor);
#endif
}

int RSC::init()
{
  int err = scnp_create_socket(&_sock, _if);

  if(err) error("Can't create socket");
  
  err = init_controller();
  if(err) error("Can't init controller");

  _run = true;
  
  return 0;
}

void RSC::exit()
{
  exit_controller();
  scnp_close_socket(&_sock);
}

void RSC::_transit(rscutil::Combo::Way way)
{
  using Way = rscutil::Combo::Way;
  
  if(way == Way::LEFT) _pc_list.previous_pc();
  else                 _pc_list.next_pc();

  grab_controller(!_pc_list.get_current().local);
 
  _state = (_pc_list.get_current().local)? State::HERE : State::AWAY;

#ifndef NO_CURSOR
  if(_state == State::AWAY) hide_cursor(_cursor);
  else                      show_cursor(_cursor);
#endif
}

#ifndef NO_CURSOR

void RSC::_transit(rscutil::Combo::Way way, float height)
{
  using Way = rscutil::Combo::Way;
  
  _transit(way);
  
  if(_pc_list.get_current().local) {
    if(_pc_list.size() > 1) {
      _cursor->pos_x = (way == Way::RIGHT)?10:_cursor->screen_size.width-10;
      _cursor->pos_y = height * _cursor->screen_size.height;
      set_cursor_position(_cursor);
    }
  }
  else {
    struct scnp_out pkt;
    pkt.type = SCNP_OUT;
    pkt.direction = OUT_INGRESS;
    pkt.side = (way == Way::LEFT)? OUT_LEFT : OUT_RIGHT;
    pkt.height = height;
    scnp_send(&_sock,
	      reinterpret_cast<scnp_packet*>(&pkt),
	      _pc_list.get_current().address);

  }
}

#endif

void RSC::add_pc(const uint8_t *addr, const std::string& hostname)
{
  using namespace rscutil;
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

void RSC::_receive()
{
  using rscutil::Combo;
  
  struct scnp_packet   packet;
  ControllerEvent    * ev = nullptr;
  uint8_t              addr_src[rscutil::PC::LEN_ADDR];
  int                  can_update = 0;
    
#ifndef NO_CURSOR
  const rscutil::PC&   local_pc = _pc_list.get_local();
  rscutil::ComboMouse  mouse(local_pc.resolution.w, local_pc.resolution.h,_cursor);
  
  mouse.set_action([&](Combo* combo) {
		     auto way = combo->get_way();
		     
		     struct scnp_out pkt;
		     pkt.type = SCNP_OUT;
		     pkt.side = (Combo::Way::RIGHT == way);
		     pkt.direction = OUT_EGRESS;
		     pkt.height = _cursor->pos_y / (float)_cursor->screen_size.height;
		     scnp_send(&_sock,
			       reinterpret_cast<struct scnp_packet *>(&pkt),
			       addr_src);
		     --can_update;
		   });
#endif

  std::map<uint8_t, std::function<ControllerEvent *(void)>> on_packet =
    {
     { SCNP_KEY, [&packet]() { return ConvKey<ControllerEvent,KEY>::get(packet); }},
     { SCNP_MOV, [&packet]() { return ConvKey<ControllerEvent,MOUSE>::get(packet); }},
     { SCNP_OUT, [&packet,this,&can_update]() {
#ifndef NO_CURSOR
		   auto * pkt = reinterpret_cast<struct scnp_out*>(&packet);
		   
		   if(pkt->direction == OUT_EGRESS) {
		     if(!pkt->side) _transit(Combo::Way::LEFT, pkt->height);
		     else           _transit(Combo::Way::RIGHT, pkt->height);		     
		   }
		   else {
		     _cursor->pos_x = (pkt->side == OUT_RIGHT)?10:_cursor->screen_size.width-10;
		     _cursor->pos_y = pkt->height * _cursor->screen_size.height;
		     set_cursor_position(_cursor);
		     ++can_update;
		   }
#endif
		   return nullptr;
		 }},
     { SCNP_MNG, [this, &addr_src, &packet]() {
		    auto * pkt = reinterpret_cast<struct scnp_management*>(&packet);
		    add_pc(addr_src, pkt->hostname);
		    return nullptr; }},
    };

  while(_run) {
    int err = scnp_recv(&_sock, &packet, addr_src);

    if(err == -1) perror("scnp_recv");
    
    auto it = on_packet.find(packet.type);
    
    if(it != on_packet.end()) ev = it->second();
    else                      ev = nullptr;
    
    if(ev && can_update) {
      write_controller(ev);

#ifndef NO_CURSOR
      if(ev->controller_type == MOUSE) mouse.update(ev->code, ev->value);
#endif
    }
  }
}


void RSC::_send(const ControllerEvent &ev)
{
  switch(ev.controller_type) {
  case MOUSE:
    scnp_send(&_sock,
	      ConvKey<struct scnp_packet, MOUSE>::get(ev),
	      _pc_list.get_current().address);
    break;
  case KEY:
    scnp_send(&_sock,
	      ConvKey<struct scnp_packet, KEY>::get(ev),
	      _pc_list.get_current().address);
    break;
  default:
    break;
  }
}

void RSC::_local_cmd()
{
  using namespace rsclocalcom;
  Message     msg, ack(Message::ACK);
  bool        pause_request = false, stop_request = false;

  std::map<Message::Command, std::function<void(const Message&)>> on_msg = 
    {
      { Message::IF, [this,&ack](const Message& m) {
	  set_interface(std::stoi(m.get_arg(0)));
	  ack.add_arg(Message::OK, Message::DEFAULT);  }},
      { Message::GETIF, [this,&ack](const Message&) {
	  ack.add_arg(Message::OK, _if);  }},
      { Message::GETLIST, [this, &ack](const Message& ) {
	  _pc_list.save(CURRENT_PC_LIST);
	  _all_pc_list.save(ALL_PC_LIST);
	  ack.add_arg(Message::OK, Message::DEFAULT); } },
      { Message::SETLIST, [this, &ack](const Message& ) {
	  _th_safe_op(_pc_list_mutex, [this]() {_pc_list.load(CURRENT_PC_LIST);});
	  _th_safe_op(_all_pc_list_mutex,[this](){_all_pc_list.load(ALL_PC_LIST);});
	  ack.add_arg(Message::OK, Message::DEFAULT);
	}},
      { Message::START, [&ack](const Message&) {
	  ack.add_arg(Message::ERROR, Message::STARTED);
	}},
      { Message::STOP, [&stop_request, &ack](const Message&) {
	  stop_request = true;
	  ack.add_arg(Message::OK, Message::DEFAULT);
	}},
      { Message::PAUSE, [&pause_request, &ack](const Message&) {
	  pause_request = true;
	  ack.add_arg(Message::OK, Message::DEFAULT);
	}},
    };

  while(_run) {
    ack.reset(Message::ACK);
   
    _com.read(msg);
    on_msg[msg.get_cmd()](msg);
    _com.send(ack);

    if(pause_request)     pause_requested();
    else if(stop_request) stop_requested();
  }

}

void RSC::_keep_alive()
{
  decltype(_alive)::iterator it;
  
  while(_run) {

    do {
      it = std::find_if(_alive.begin(),
			_alive.end(),
			[](const auto& a) {
			  auto now = RSC::clock_t::now();
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

void RSC::_send()
{
  ControllerEvent c;
  
  while(_run) {
    c.grabbed = _state == State::AWAY;
    
    int ret = poll_controller(&c);
    if(!ret) continue;
    
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

void RSC::run()
{
  scnp_start_session(_if);
  
  _pause = false;
  _run = true;
  
  _threads.push_back(std::thread(std::bind(&RSC::_receive, this)));
  _threads.push_back(std::thread([this]() { _send(); }));
  _threads.push_back(std::thread(std::bind(&RSC::_keep_alive, this)));
  _threads.push_back(std::thread(&RSC::_local_cmd, this));

  for(auto&& th : _threads) th.join();

  _threads.clear();
  scnp_stop_session(_if);
}

void RSC::pause_requested()
{
  _pause = true;
  for(auto&& th : _threads) pthread_cancel(th.native_handle());  
}

void RSC::stop_requested()
{
  _run = false;
  for(auto&& th : _threads) pthread_cancel(th.native_handle());
}

void RSC::set_interface(int index)
{
  scnp_stop_session(_if);
  _if = index;
  _sock.if_index = index;
  scnp_start_session(_if);
}

void RSC::wait_for_wakeup()
{
  using namespace rsclocalcom;
  Message msg, ack(Message::ACK);

  while(_pause) {
    _com.read(msg);
    ack.reset(Message::ACK);

    switch(msg.get_cmd()) {
    case Message::START:
      _pause = false;
      ack.add_arg(Message::OK, Message::DEFAULT);
      break;
    case Message::STOP:
      _pause = false;
      _run = false;
      ack.add_arg(Message::OK, Message::DEFAULT);
      break;
    default:
      ack.add_arg(Message::ERROR, Message::PAUSED);
      break;
    }

    _com.send(ack);
  }
}

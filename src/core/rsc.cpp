#include <functional>
#include <string>
#include <cstring>
#include <algorithm>

#include <controller.h> // Must be before convkey.hpp
#include <rsc.hpp> // Also before convkey.hpp

#include <convkey.hpp>
#include <config.hpp>
#include <interface.h>

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

void RSC::save_shortcut() const
{
  using rscutil::ComboShortcut;
  ComboShortcut::ComboShortcutList list;
  
  for(const auto& a: _shortcut) {
    if(a->get_type() == ComboShortcut::TYPE) {
      list.push_back(*static_cast<ComboShortcut*>(a.get()));
    }
  }

  ComboShortcut::save(list);
}

void RSC::_send_release(rscutil::Combo* combo)
{
  using rscutil::ComboShortcut;
  
  auto * c = static_cast<ComboShortcut *>(combo);

  c->for_each([this](ComboShortcut::shortcut_t& s) {
      int code = std::get<0>(s);

      ControllerEvent e = { false, KEY, EV_KEY, KEY_RELEASED, 0};
      e.code = code;

      _send(e);
    });
}

void RSC::load_shortcut(bool reset)
{
  using rscutil::Combo;
  using rscutil::ComboShortcut;

  std::map<std::string, std::function<void(Combo*)>> _actions = {
    { "right", [this](Combo * combo) { _send_release(combo); _transit(combo->get_way()); } },
    { "left", [this](Combo * combo) { _send_release(combo); _transit(combo->get_way()); } },
    { "quit", [this](Combo*) { _run = false; } }
  };

  _shortcut.erase(std::remove_if(_shortcut.begin(),
				 _shortcut.end(),
				 [](auto&& a) { return a->get_type() == ComboShortcut::TYPE; }),
		  _shortcut.end());

  ComboShortcut::ComboShortcutList list;
  bool                             success = false;

  if(!reset) success = ComboShortcut::load(list);
  
  if(success) {
    for(auto& combo : list) {
      combo.set_action(_actions[combo.get_name()]);
      _shortcut.push_back(std::make_unique<ComboShortcut>(combo));
    }
  }
  else {
    // Default shortcut
    auto right = ComboShortcut::make_ptr("right", "Move to the next computer on the right");

    right->add_shortcut(KEY_LEFTCTRL, KEY_PRESSED);
    right->add_shortcut(KEY_R, KEY_PRESSED);
    right->add_shortcut(KEY_RIGHT, KEY_PRESSED);  
    right->release_for_all();
    right->set_action(_actions["right"]);

    auto left = ComboShortcut::make_ptr("left", "Move to the next computer on the left",
					Combo::Way::LEFT);

    left->add_shortcut(KEY_LEFTCTRL, KEY_PRESSED);
    left->add_shortcut(KEY_R, KEY_PRESSED);
    left->add_shortcut(KEY_LEFT, KEY_PRESSED);  
    left->release_for_all();
    left->set_action(_actions["left"]);
  
    auto quit = ComboShortcut::make_ptr("quit", "Quit the service", Combo::Way::NONE);

    for(int i = 0; i < 3; i++) {
      quit->add_shortcut(KEY_ESC, KEY_PRESSED, 200);
      quit->add_shortcut(KEY_ESC, KEY_RELEASED, 200);
    }
  
    quit->set_action(_actions["quit"]);

    _shortcut.push_back(std::move(right));
    _shortcut.push_back(std::move(left));
    _shortcut.push_back(std::move(quit));

    if(reset) save_shortcut();
  }
}

RSC::RSC(): _if(DEFAULT_IF), _next_pc_id{0},
	      _com(rsclocalcom::RSCLocalCom::Contact::CORE),
	      _state(State::HERE)
{
  using namespace rscutil;
  
  load_shortcut(false);
  
  PC local_pc { _next_pc_id++, true, true, "localhost", {0}, {0,0}, {0,0}};

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

int RSC::init(int if_index)
{
  _if = if_index;
  int err = scnp_start(_if);

  if(err) error("Cannot start SCNP session");
  
  err = init_controller();
  if(err) error("Cannot init controller");

  _run = true;
  
  return 0;
}

void RSC::exit()
{
  exit_controller();
  scnp_stop();
}

void RSC::_transit(rscutil::Combo::Way way)
{
  using Way = rscutil::Combo::Way;

  _pc_list.get_current().focus = false;
  
  if(way == Way::LEFT)       _pc_list.previous_pc();
  else if(way == Way::RIGHT) _pc_list.next_pc();

  _pc_list.get_current().focus = true;
  
  grab_controller(!_pc_list.get_current().local);
 
  _state = (_pc_list.get_current().local)? State::HERE : State::AWAY;

  _waiting_for_egress = _state != State::HERE;
  
#ifndef NO_CURSOR
  if(_state == State::AWAY) hide_cursor(_cursor);
  else                      show_cursor(_cursor);

  _cursor->pos_x = _cursor->screen_size.width >> 1;
  _cursor->pos_y = _cursor->screen_size.height >> 1;
  set_cursor_position(_cursor);
#endif
}

#ifndef NO_CURSOR

void RSC::_transit(rscutil::Combo::Way way, float height)
{
  using Way = rscutil::Combo::Way;
  
  _pc_list.get_current().focus = false;
  
  if(way == Way::LEFT)       _pc_list.previous_pc();
  else if(way == Way::RIGHT) _pc_list.next_pc();

  _pc_list.get_current().focus = true;

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
    scnp_send(reinterpret_cast<scnp_packet*>(&pkt),
	      _pc_list.get_current().address);
    _waiting_for_egress = true;
  }

  _state = (_pc_list.get_current().local)? State::HERE : State::AWAY;

  if(_state == State::AWAY) hide_cursor(_cursor);
  else                      show_cursor(_cursor);

  // grab controller is very slow
  grab_controller(_state == State::AWAY);
}

#endif

void RSC::add_pc(const uint8_t *addr, const std::string& hostname)
{
  using namespace rscutil;
  bool exist = _all_pc_list.exist([&addr](const PC& pc) -> bool {
				    return !memcmp(addr, pc.address, PC::LEN_ADDR);
				  });
  if(!exist) {
    PC pc{ _next_pc_id++, false, false, hostname, {0}, { 0,0 }, { 0,0 }}; 

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
		     scnp_send(reinterpret_cast<struct scnp_packet *>(&pkt), addr_src);
		   });
#endif

  std::map<uint8_t, std::function<ControllerEvent *(void)>> on_packet =
    {
     { SCNP_KEY, [&packet]() { return ConvKey<ControllerEvent,KEY>::get(packet); }},
     { SCNP_MOV, [&packet]() { return ConvKey<ControllerEvent,MOUSE>::get(packet); }},
     { SCNP_OUT, [&packet,this]() {
#ifndef NO_CURSOR
		   auto * pkt = reinterpret_cast<struct scnp_out*>(&packet);
		   
		   if(pkt->direction == OUT_EGRESS) {
		     if(_waiting_for_egress) _waiting_for_egress = false;
		     else                    return nullptr;
		     
		     if(!pkt->side) _transit(Combo::Way::LEFT, pkt->height);
		     else           _transit(Combo::Way::RIGHT, pkt->height);		     
		   }
		   else {
		     _cursor->pos_x = (pkt->side == OUT_RIGHT)?10:_cursor->screen_size.width-10;
		     _cursor->pos_y = pkt->height * _cursor->screen_size.height;
		     set_cursor_position(_cursor);
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
    int err = scnp_recv(&packet, addr_src);

    if(err == -1) perror("scnp_recv");
    
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


void RSC::_send(const ControllerEvent &ev)
{
  switch(ev.controller_type) {
  case MOUSE:
    scnp_send(ConvKey<struct scnp_packet, MOUSE>::get(ev),
	      _pc_list.get_current().address);
    break;
  case KEY:
    scnp_send(ConvKey<struct scnp_packet, KEY>::get(ev),
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
	  int ret = set_interface(std::stoi(m.get_arg(0)));
	  if(ret) ack.add_arg(Message::ERROR, Message::IF_EXIST);
	  else    ack.add_arg(Message::OK, Message::DEFAULT);  }},
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
      { Message::SAVE_SHORTCUT, [this, &ack](const Message&) {
	  save_shortcut();
	  ack.add_arg(Message::OK, Message::DEFAULT);
	}},
      { Message::LOAD_SHORTCUT, [this, &ack](const Message& msg) {
	  int arg = std::stoi(msg.get_arg(0));
	  load_shortcut(arg == Message::LOAD_RESET);
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
    
    int ret = poll_controller(&c, -1);
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
  _pause = false;
  _run = true;
  
  _threads.push_back(std::thread(std::bind(&RSC::_receive, this)));
  _threads.push_back(std::thread([this]() { _send(); }));
  _threads.push_back(std::thread(std::bind(&RSC::_keep_alive, this)));
  _threads.push_back(std::thread(&RSC::_local_cmd, this));

  for(auto&& th : _threads) th.join();

  _threads.clear();
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

int RSC::set_interface(int index)
{
  IF * interfaces = get_interfaces();
  IF * i = interfaces;

  while(!(i->if_name == nullptr && i->if_index == 0) && i->if_index != (unsigned)index)
    ++i;

  bool not_found = i->if_name == nullptr && i->if_index == 0;

  free(interfaces);
  i = nullptr;
  interfaces = nullptr;
  
  if(not_found) return 1;
  
  scnp_stop();
  _if = index;
  if (scnp_start(_if)) return -1;

  return 0;
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

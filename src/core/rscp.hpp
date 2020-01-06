#ifndef RSCP_H
#define RSCP_H

#include <atomic>

#include <combo.hpp>
#include <pc_list.hpp>
#include <scnp.h>

struct ControllerEvent;
struct CursorInfo;

class RSCP
{
  using socket_t = struct scnp_socket;

  static constexpr int DEFAULT_IF = 2;
  
  std::list<Combo::ptr> _shortcut;
  PCList                _pc_list;
  PCList                _all_pc_list;
  socket_t              _sock;
  std::atomic_bool      _run;
  int                   _if;
  int                   _next_pc_id;
  CursorInfo *          _cursor;
  
  void _receive();
  void _local_cmd();
  void _send(const ControllerEvent& ev);
  void _transit(Combo::Way way);

public:

  enum class State { HERE, AWAY };
  
  RSCP();
  ~RSCP();

  void run();
  int  init();
  void exit();
  void add_pc(const uint8_t addr[]);

  const PCList&  get_config() const { return _pc_list; }
  PCList&        get_config() { return _pc_list; }
  
  void set_interface(int index) { _if = index; }
  
private:
  State _state;
};

#endif /* RSCP_H */

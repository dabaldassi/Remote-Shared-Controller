#ifndef RSCP_H
#define RSCP_H

#include <combo.hpp>
#include <pc_list.hpp>
#include <scnp.h>
#include <atomic>

struct ControllerEvent;

class RSCP
{
  using socket_t = struct scnp_socket;

  static constexpr int DEFAULT_IF = 2;
  
  Combo::ptr         _swap;
  ComboShortcut      _quit_shortcut;
  PCList             _pc_list;
  socket_t           _sock;
  std::atomic_bool   _run;
  bool               _transition;
  int                _if;
  
  void _receive();
  void _send(const ControllerEvent& ev);
  void _transit();

public:

  enum class State { HERE, AWAY };
  
  RSCP();
  virtual ~RSCP() = default;

  void run();
  int  init();
  void exit();

  const PCList&  get_config() const { return _pc_list; }
  PCList&        get_config() { return _pc_list; }
  
  void set_interface(int index) { _if = index; }
  
private:
  State _state;
};

#endif /* RSCP_H */

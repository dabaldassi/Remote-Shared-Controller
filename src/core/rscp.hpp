#ifndef RSCP_H
#define RSCP_H

// #include <swap_screen.hpp>
// #include <pc_list.hpp>
#include <network.h>
#include <atomic>

struct ControllerEvent;

class RSCP
{
  using socket_t = struct scnp_socket;

  static constexpr int IF = 16;
  
  // SwapScreen::ptr  _swap;
  // PCList           _pc_list;
  socket_t         _sock;
  std::atomic_bool _run;
  bool             _transition;
  
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

  // PCList::ptr    refresh();
  // const PCList&  get_config() const { return _pc_list; }
  // void           set_config(PCList& list);
  // void           set_swap_screen(SwapScreen& s);

private:
  State _state;
};

#endif /* RSCP_H */

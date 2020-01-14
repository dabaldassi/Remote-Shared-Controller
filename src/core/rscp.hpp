#ifndef RSCP_H
#define RSCP_H

#include <atomic>
#include <chrono>
#include <map>
#include <mutex>

#include <combo.hpp>
#include <pc_list.hpp>
#include <scnp.h>

#ifndef NO_CURSOR
#include <cursor.h>
#endif

struct ControllerEvent;

class RSCP
{
  using socket_t = struct scnp_socket;

  static constexpr int DEFAULT_IF = 2;
  static constexpr int ALIVE_TIMEOUT = 5;

  using clock_t = std::chrono::system_clock;
  using timestamp_t = std::chrono::time_point<RSCP::clock_t>;
  
  std::map<int, timestamp_t> _alive;
  
  std::list<Combo::ptr> _shortcut;
  PCList                _pc_list;
  PCList                _all_pc_list;
  socket_t              _sock;
  std::atomic_bool      _run;
  int                   _if;
  int                   _next_pc_id;
  CursorInfo *          _cursor;

  std::mutex _pc_list_mutex;
  std::mutex _all_pc_list_mutex;
  std::mutex _alive_mutex;

  /**
   *\brief Lock a mutex to execute safely an operation
   *\param m The mutex to lock/unlock
   *\param l The operation to execute in a lambda.
   */
  
  template<typename Mutex, typename Lambda>
  void _th_safe_op(Mutex& m, Lambda && l);

  /**
   *\brief Listening thread to sncp_packet
   */
  
  void _receive();

  /**
   *\biref Listening thread to local command
   */
  
  void _local_cmd();

  /**
   *\brief Thread to erase all pc that have tiemout
   */
  
  void _keep_alive();

  /**
   *\brief Listening thread to event. Then forward them through sncp packet
   */
  
  void _send(const ControllerEvent& ev);

  /**
   *\brief Get the next pc in the list.
   *\param way If this is the next or previous pc.
   */
  
  void _transit(Combo::Way way);

public:

  enum class State { HERE, AWAY };
  
  RSCP();
  ~RSCP();

  /**
   *\brief run an rsc instance
   */
  
  void run();

  /**
   *\brief Init the object.
   *\return 1 if there was an error. 0 otherwise.
   */
  
  int  init();

  /**
   *\brief Release everything that need to be in the object
   */
  
  void exit();

  /**
   *\brief Add a PC to the list (all_pc) by its mac address
   *\param addr The mac address of the pc
   */
  
  void add_pc(const uint8_t addr[]);
  
  void set_interface(int index) { _if = index; }
  
private:
  State _state;
};

#endif /* RSCP_H */

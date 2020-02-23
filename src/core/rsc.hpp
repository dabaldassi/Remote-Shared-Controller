#ifndef RSCP_H
#define RSCP_H

#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <thread>

#include <rsclocal_com.hpp>
#include <combo.hpp>
#include <pc_list.hpp>
#include <scnp.h>

#ifndef NO_CURSOR
#include <cursor.h>
#endif

struct ControllerEvent;

class RSC
{
  using clock_t = std::chrono::system_clock;
  using timestamp_t = std::chrono::time_point<RSC::clock_t>;

  static constexpr int DEFAULT_IF = 5;
  static constexpr int ALIVE_TIMEOUT = 5;
  
  std::map<int, timestamp_t> _alive;
  
  std::list<rscutil::Combo::ptr> _shortcut;
  rscutil::PCList                _pc_list;
  rscutil::PCList                _all_pc_list;
  std::atomic_bool               _run, _pause;
  std::pair<bool, uint8_t[6]>    _waiting_for_egress;
  int                            _if;
  int                            _next_pc_id;
  CursorInfo *                   _cursor;
  
  rsclocalcom::RSCLocalCom _com;
  std::vector<std::thread> _threads; // Index 0 is reserved for keep_alive
  std::mutex               _pc_list_mutex;
  std::mutex               _all_pc_list_mutex;
  std::mutex               _alive_mutex;
  std::mutex               _state_mutex;
  std::mutex               _cursor_mutex;
  std::mutex               _egress_mutex;

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
   *\brief Send an event through scnp_packet.
   *\param ev The evenement to send
   */
  
  void _send(const ControllerEvent& ev);

  /**
   *\brief Listening thread to event.
   */
  
  void _send();

  /**
   *\brief Get the next pc in the list.
   *\param way If this is the next or previous pc.
   */

  void _send_release(rscutil::Combo* s);
  
  void _transit(rscutil::Combo::Way way);

  #ifndef NO_CURSOR
  void _transit(rscutil::Combo::Way way, float height);
  #endif

public:

  enum class State { HERE, AWAY };
  
  RSC();
  ~RSC();

  /**
   *\brief run an rsc instance
   */
  
  void run();

  /**
   *\brief Init the object.
   *\return 1 if there was an error. 0 otherwise.
   */
  
  int  init(int if_indexma);

  /**
   *\brief Release everything that need to be in the object
   */
  
  void exit();

  /**
   *\brief Add a PC to the list (all_pc) by its mac address
   *\param addr The mac address of the pc
   */
  
  void add_pc(const uint8_t addr[], const std::string& hostname);

  /**
   *\brief Change the current network interface.
   *\param index The index of the current interface.
   *\return 0 on success. 1 otherwise.
   */
  
  int set_interface(int index);

  /**
   *\brief Put rsc core into sleep.
   */
  
  void pause_requested();
  
  /**
   *\brief Stop rsc core.
   */
  
  void stop_requested();
  
  bool is_paused() const { return _pause; }
  bool is_running() const { return _run; }

  /**
   *\brief Wait for a command to wake up the core or stop it when sleeping.
   */
  
  void wait_for_wakeup();

  void save_shortcut() const;
  void load_shortcut(bool reset);
  
private:
  State _state;
};

#endif /* RSCP_H */

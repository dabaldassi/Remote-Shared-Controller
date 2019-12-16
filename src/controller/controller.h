#ifndef EVENT_INTERFACE_H
#define EVENT_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__gnu_linux__)
#include <linux/input-event-codes.h>  
#endif

  /*
   * Id for controller type
   */
  
#define KEY 0x01
#define MOUSE 0x02

  /*
   * Possible value for key event
   */
  
#define KEY_RELEASED 0x00
#define KEY_PRESSED 0x01
  
  typedef struct ControllerEvent
  {
    uint8_t        controller_type;  // Which controler it is
    uint8_t        ev_type;          // Event types
    int32_t        value;            // If a key is pressed or released, coord mouse
    unsigned short code;             // the code corresponding to the event
  }ControllerEvent;

  /**
   *\brief Simulate the event represented by the ControllerEvent structure
   *\param ce A pointer to the event. Must not be NULL.
   */

  void write_controller(const ControllerEvent * ce);

  /**
   *\brief Simulate a key (keyboard or click) with key pressed and then key event
   *\param c The key code
   */
  
  void write_key(unsigned char c);

  /**
   *\brief Simulate a key with the sent value. Either pressed or released
   *\param c The key code
   *\param mode If the event is pressed or released
   */

  void write_key_ev(unsigned char c, int mode);

  /**
   *\brief Get the code of a key when an event occur.
   *\param key_code A pointer to store the key code
   *\param value A pointer to store the value (pressed or released)
   *\return true if there is something to read. False otherwise
   */
  
  bool get_key(unsigned short * key_code, int * value);

  /**
   *\brief Mouve the mouse with relative event
   *\param x The relative position on the X axis (horizontally)
   *\param y The relative position on the Y axis (vertivally)
   */
  
  void mouse_move(int x, int y);

  /**
   *\brief Wait for an event to occur and store it in the ControllerEvent parameter
   *\param ev The event that just happened
   *\return 1 if there is something to read, negative value if error, 0 otherwise
   */
  
  int poll_controller(ControllerEvent * ev);

  /**
   *\brief Make all the controller events only readable by the current process. All other process (as X server for example) can not read event anymore.
   *\param t True fro grabbing, False for releasing
   */
  
  void grab_controller(bool t);

  /**
   *\brief Init all data needed to use the event in the program.
   *\return 0 on success, the error otherwise.
   */
  
  int init_controller(void);

  /**
   *\brief Release everything related to the controller
   */
  
  void exit_controller(void);

#ifdef __cplusplus  
}
#endif

#endif /* EVENT_INTERFACE_H */

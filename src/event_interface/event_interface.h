#ifndef EVENT_INTERFACE_H
#define EVENT_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__gnu_linux__)
#include <linux/input-event-codes.h>
#define TRIGGER ABS_MT_TRACKING_ID
  
#endif

#define KEY 0x01
#define MOUSE 0x02

#define KEY_RELEASED 0
#define KEY_PRESSED 1
#define ABS_ENTER 2
#define ABS_EXIT  3
  
  typedef struct ControllerEvent
  {
    uint8_t       controller_type;
    uint8_t       ev_type;
    uint32_t      value;
    unsigned short code; // Key if keyboard
  }ControllerEvent;

  void write_controller(const ControllerEvent * ce);
  void write_key(unsigned char c);
  void write_key_ev(unsigned char c, int mode);
  bool get_key(unsigned short * key_code, int * value); // non block
  void mouse_move(int x, int y);

  int poll_controller(ControllerEvent *);
  void grab_controller(bool t); // Activate or desactivate keyboard

  int init_controller(void);
  void exit_controller(void);

#ifdef __cplusplus  
}
#endif

#endif /* EVENT_INTERFACE_H */

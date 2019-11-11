#ifndef EVENT_INTERFACE_H
#define EVENT_INTERFACE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__gnu_linux__)
#include <linux/input-event-codes.h>
#endif

#define KEYBOARD_EV 0x01
#define MOUSE_EV    0x02

#define EV_RELEASED 0
#define EV_PRESSED  1

typedef struct ControllerEvent
{
  u_int8_t      controller_type;
  unsigned char key; // Key if keyboard
}ControllerEvent;

void write_key(unsigned char c);
unsigned char get_key(void); // block

int poll(ControllerEvent *);
void set_state_keyboard(bool t); // Activate or desactivate keyboard

int init_controller(void);
void exit_controller(void);

#ifdef __cplusplus  
}
#endif

#endif /* EVENT_INTERFACE_H */

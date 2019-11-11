#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>

#include "event_interface.h"

static int uinput_file_descriptor = -1;

static void emit(int fd, int type, int code, int val)
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}

void write_key(unsigned char c)
{
  emit(uinput_file_descriptor, EV_KEY, c, EV_PRESSED);
  emit(uinput_file_descriptor, EV_SYN, SYN_REPORT, 0);
  emit(uinput_file_descriptor, EV_KEY, c, EV_RELEASED);
  emit(uinput_file_descriptor, EV_SYN, SYN_REPORT, 0);
}

int init_controller(void)
{
  struct uinput_setup usetup;

  if(uinput_file_descriptor == -1) {
    uinput_file_descriptor = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    if(uinput_file_descriptor < 0) {
      fprintf(stderr, "Error opening uinput");
      return 1; // error opening
    }

    /*
     * The ioctls below will enable the device that is about to be
     * created, to pass key events.
     */
    ioctl(uinput_file_descriptor, UI_SET_EVBIT, EV_KEY);

    /*
     * Enable all keyboard keys
     */
    for(int i = 0; i < 255; ++i) {
      ioctl(uinput_file_descriptor, UI_SET_KEYBIT, i);
    }

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0xcafe; /* sample vendor */
    usetup.id.product = 0xbabe; /* sample product */
    strcpy(usetup.name, "Shared Controller");

    ioctl(uinput_file_descriptor, UI_DEV_SETUP, &usetup);
    ioctl(uinput_file_descriptor, UI_DEV_CREATE);

    return 0; // OK
  }

  return 1; // Error file already open
}

void exit_controller(void)
{
  if(uinput_file_descriptor != -1) {
    ioctl(uinput_file_descriptor, UI_DEV_DESTROY);
    close(uinput_file_descriptor);
    uinput_file_descriptor = -1;
  }
}

void set_state_keyboard(bool t)
{
  
}

int poll(ControllerEvent * ce)
{

}

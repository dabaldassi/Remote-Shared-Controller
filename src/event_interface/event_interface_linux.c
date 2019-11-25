#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <poll.h>
#include <dirent.h>
#include <unistd.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include "event_interface.h"

#define DEV_INPUT_FILE_NAME "/dev/input/"

static int uinput_file_descriptor = -1;
static struct pollfd * event_poll_file_descriptor = NULL;

static void emit(int fd, int type, unsigned short code, int val)
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
  emit(uinput_file_descriptor, EV_KEY, c, KEY_PRESSED);
  emit(uinput_file_descriptor, EV_SYN, SYN_REPORT, 0);
  emit(uinput_file_descriptor, EV_KEY, c, KEY_RELEASED);
  emit(uinput_file_descriptor, EV_SYN, SYN_REPORT, 0);
}

void write_key_ev(unsigned char c, int mode)
{
  emit(uinput_file_descriptor, EV_KEY, c, mode);
  emit(uinput_file_descriptor, EV_SYN, SYN_REPORT, 0);
}

void mouse_move(int x, int y)
{
  emit(uinput_file_descriptor, EV_REL, REL_X, x);
  emit(uinput_file_descriptor, EV_REL, REL_Y, y);
  emit(uinput_file_descriptor, EV_SYN, SYN_REPORT, 0);
}

bool get_key(unsigned short * code, int * val)
{
  struct input_event ie;

  int i = 0, run = 1;

  while(run && event_poll_file_descriptor[i].fd > 0) {
    read(event_poll_file_descriptor[i].fd, &ie, sizeof(ie));
    if(ie.type == EV_KEY) {
      *code = ie.code;
      *val = ie.value;
      read(event_poll_file_descriptor[i].fd, &ie, sizeof(ie));
      // run = 0;
      run = !(ie.code == SYN_REPORT && ie.type == EV_SYN && ie.value == 0);
    }
    ++i;
  }
  
  return !run;
}

static int init_uinput(void)
{
  struct uinput_setup usetup;
  
  const char uinput_file[] = "/dev/uinput";
  const char device_name[] = "Shared Controller";

  uinput_file_descriptor = open(uinput_file, O_WRONLY | O_NONBLOCK);

  if(uinput_file_descriptor < 0) return 1; // error opening
  
  /*
   * The ioctls below will enable the device that is about to be
   * created, to pass key events.
   */
  ioctl(uinput_file_descriptor, UI_SET_EVBIT, EV_KEY);
  ioctl(uinput_file_descriptor, UI_SET_KEYBIT, BTN_LEFT);
  ioctl(uinput_file_descriptor, UI_SET_KEYBIT, BTN_RIGHT);
  ioctl(uinput_file_descriptor, UI_SET_EVBIT, EV_REL);
  ioctl(uinput_file_descriptor, UI_SET_RELBIT, REL_X);
  ioctl(uinput_file_descriptor, UI_SET_RELBIT, REL_Y);

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
  strcpy(usetup.name, device_name);

  ioctl(uinput_file_descriptor, UI_DEV_SETUP, &usetup);
  ioctl(uinput_file_descriptor, UI_DEV_CREATE);

  return 0; // OK
}

static int init_read(void)
{  
  const char   event_file_prefix[] = "event";
  const size_t len_event = strlen(event_file_prefix);
  const size_t base_len = 50;

  DIR           * dev_input_dir = NULL;
  struct dirent * current_ptr_dir = NULL;

  int next = 0;

  dev_input_dir = opendir(DEV_INPUT_FILE_NAME);
  if(dev_input_dir == NULL) return 1; // Error opening

  event_poll_file_descriptor = calloc(sizeof(int), base_len);
  if(event_poll_file_descriptor == NULL) {
    perror("");
    return 1;
  }

  while ((current_ptr_dir = readdir(dev_input_dir))) {
    if(!strncmp(current_ptr_dir->d_name,event_file_prefix,len_event)) {
      char dir_name[256] = DEV_INPUT_FILE_NAME;
      
      event_poll_file_descriptor[next].fd = open(strcat(dir_name, current_ptr_dir->d_name),
						 O_RDONLY | O_NONBLOCK);
      event_poll_file_descriptor[next].events = POLLIN;
      
      if(event_poll_file_descriptor[next].fd < 0) {
	fprintf(stderr, "Failed to open %s: ", dir_name);
	perror("");
      }
      
      ++next;
    }
  }

  closedir(dev_input_dir);
  return 0;
}

int init_controller(void)
{
  if(uinput_file_descriptor == -1 && event_poll_file_descriptor == NULL) {
    if(init_uinput()) {
      fprintf(stderr, "Failed to init uinput module\n");
      return 1;
    }

    if(init_read()) {
      fprintf(stderr, "Failed to read event files\n");
      return 1;
    }
    
    return 0;
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

  if(event_poll_file_descriptor) {
    int i = 0;

    while(event_poll_file_descriptor[i].fd > 0) {
      close(event_poll_file_descriptor[i].fd);
      event_poll_file_descriptor[i].fd = 0;
      ++i;
    }

    free(event_poll_file_descriptor);
    event_poll_file_descriptor = NULL;
  }
}

void grab_controller(bool t)
{
  int i = 1;
  
  while(event_poll_file_descriptor[i].fd > 0) {
    ioctl(event_poll_file_descriptor[i].fd, EVIOCGRAB, t);
    ++i;
  }
}

static inline size_t get_event_len()
{
  size_t i = 0;
  while(event_poll_file_descriptor[i].fd > 0) ++i;
  return i;
}


void write_controller(const ControllerEvent * ce)
{
  emit(uinput_file_descriptor, ce->ev_type, ce->code, ce->value);
}

int poll_controller(ControllerEvent * ce)
{
  size_t event_len = get_event_len();

  int p = poll(event_poll_file_descriptor, event_len, -1);
  if(p < 0) return -1; // error

  size_t i = 0;
  int    quit = 0;
  struct input_event ie;
  
  while(p > 0 && i < event_len && !quit) {
    if(event_poll_file_descriptor[i].revents == POLLIN) {
      read(event_poll_file_descriptor[i].fd, &ie, sizeof(ie));
      
      if(ie.type == EV_KEY || ie.type == EV_REL || ie.type == EV_ABS) {
	ce->ev_type = ie.type;
	ce->code = ie.code;
	ce->value = ie.value;
	quit = 1;
	--p;
      }
    }

    ++i;
  }
  
  return !quit;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <poll.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <errno.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include "controller.h"

#define DEV_INPUT_FILE_NAME "/dev/input/"
#define BASE_LEN 50

typedef struct EventFileInfo
{
  size_t           capacity;
  size_t           size;
  int              wd; // Watch directory (i.e : /dev/input)
  char          ** names;
  struct pollfd *  pfds;
}EventFileInfo;

static int           uinput_file_descriptor = -1;
static EventFileInfo event_file_info;

static void add_event_file(const char * name, bool must_grab)
{
  if(event_file_info.size >= event_file_info.capacity) {
    event_file_info.capacity <<= 1;
    char ** names_tmp = malloc(sizeof(char*) * event_file_info.capacity);
    struct pollfd * pfds_tmp = malloc(sizeof(struct pollfd) * event_file_info.capacity);

    for(size_t i = 0; i < event_file_info.size; ++i) {
      names_tmp[i] = event_file_info.names[i];
      pfds_tmp[i] = event_file_info.pfds[i];
    }

    free(event_file_info.names);
    free(event_file_info.pfds);

    event_file_info.names = names_tmp;
    event_file_info.pfds = pfds_tmp;
  }
  
  event_file_info.names[event_file_info.size] = calloc(sizeof(char), strlen(name) + 1u);
  strcpy(event_file_info.names[event_file_info.size], name);

  char dir_name[256] = DEV_INPUT_FILE_NAME;
  int  fd = open(strcat(dir_name,name),
		 O_RDONLY | O_NONBLOCK);
  
  if(fd < 0) perror(dir_name);

  if(must_grab) ioctl(fd, EVIOCGRAB, 1);

  event_file_info.pfds[event_file_info.size].fd = fd;
  event_file_info.pfds[event_file_info.size].events = POLLIN;
  ++event_file_info.size;
}

static void remove_event_file(const char * name)
{
  size_t index = 1;

  while(index < event_file_info.size && strcmp(name, event_file_info.names[index])) index++;

  if(index < event_file_info.size) {
    free(event_file_info.names[index]);

    event_file_info.names[index] = NULL;
    
    for(size_t i = index; i < event_file_info.size-1; i++) {
      event_file_info.names[i] = event_file_info.names[i + 1];
      event_file_info.pfds[i] = event_file_info.pfds[i + 1];
    }

    --event_file_info.size;
  }
}

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

  while(run && event_file_info.pfds[i].fd > 0) {
    read(event_file_info.pfds[i].fd, &ie, sizeof(ie));
    if(ie.type == EV_KEY) {
      *code = ie.code;
      *val = ie.value;
      read(event_file_info.pfds[i].fd, &ie, sizeof(ie));
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

  /*
   * Enable all keyboard keys
   */
  for(int i = 0; i < KEY_CNT; ++i) {
    ioctl(uinput_file_descriptor, UI_SET_KEYBIT, i);
  }

  ioctl(uinput_file_descriptor, UI_SET_EVBIT, EV_REL);
  ioctl(uinput_file_descriptor, UI_SET_RELBIT, REL_X);
  ioctl(uinput_file_descriptor, UI_SET_RELBIT, REL_Y);
  ioctl(uinput_file_descriptor, UI_SET_RELBIT, REL_WHEEL);

  /*
   * [BUG] Setting abs bit prevent writing key
  ioctl(uinput_file_descriptor, UI_SET_EVBIT, EV_ABS);
  ioctl(uinput_file_descriptor, UI_SET_ABSBIT, ABS_X);
  ioctl(uinput_file_descriptor, UI_SET_ABSBIT, ABS_Y);
  */

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
  const size_t base_len = BASE_LEN;

  DIR           * dev_input_dir = NULL;
  struct dirent * current_ptr_dir = NULL;

  int next = 1; // One because 0 is for inotify fd
  
  event_file_info.pfds = calloc(sizeof(struct pollfd), base_len);
  event_file_info.names = calloc(sizeof(char*), base_len);
  event_file_info.size = 1u;
  event_file_info.capacity = base_len;
  
  if(event_file_info.pfds == NULL || event_file_info.names == NULL) {
    perror("");
    return 1;
  }

  /* Create the file descriptor for accessing the inotify API */
  event_file_info.pfds[0].fd = inotify_init1(IN_NONBLOCK);
  event_file_info.pfds[0].events = POLLIN;
  if(event_file_info.pfds[0].fd == -1) {
    perror("inotify_init1");
    return 1;
  }
  
  dev_input_dir = opendir(DEV_INPUT_FILE_NAME); // Open /dev/input directory
  if(dev_input_dir == NULL) return 1; // Error opening

  // Add each event file in the list
  while ((current_ptr_dir = readdir(dev_input_dir))) {
    if(!strncmp(current_ptr_dir->d_name,event_file_prefix,len_event)) {
      char dir_name[256] = DEV_INPUT_FILE_NAME;
      
      event_file_info.pfds[next].fd = open(strcat(dir_name, current_ptr_dir->d_name),
						 O_RDONLY | O_NONBLOCK);
      
      if(event_file_info.pfds[next].fd < 0) {
	fprintf(stderr, "Failed to open %s: ", dir_name);
	perror("");
      }
      else {
	event_file_info.pfds[next].events = POLLIN;
	event_file_info.names[next] = calloc(sizeof(char), strlen(current_ptr_dir->d_name)+1);
	strcpy(event_file_info.names[next], current_ptr_dir->d_name);
	++event_file_info.size;
	++next;
      }
    }
  }

  closedir(dev_input_dir);
  return 0;
}

int init_controller(void)
{
  if(uinput_file_descriptor == -1 && event_file_info.pfds == NULL) {
    
    if(init_read()) {
      fprintf(stderr, "Failed to read event files\n");
      return 1;
    }

    if(init_uinput()) {
      fprintf(stderr, "Failed to init uinput module\n");
      return 1;
    }

    /* Mark directories for events
       - file is created
       - file is deleted */

    event_file_info.wd = inotify_add_watch(event_file_info.pfds[0].fd,
					   DEV_INPUT_FILE_NAME,
					   IN_DELETE | IN_CREATE);
    if(event_file_info.wd == -1) {
      perror("inotify_add_watch");
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

  if(event_file_info.pfds) {

    close(event_file_info.pfds[0].fd);
    event_file_info.pfds[0].fd = -1;
    
    for(size_t i = 1; i < event_file_info.size; ++i) {
      close(event_file_info.pfds[i].fd);
      event_file_info.pfds[i].fd = -1;
      free(event_file_info.names[i]);
      event_file_info.names[i] = NULL;
    }

    free(event_file_info.names);
    free(event_file_info.pfds);
    event_file_info.pfds = NULL;
    event_file_info.names = NULL;
    event_file_info.size = 0u;
  }
}

void grab_controller(bool t)
{
  static bool grabbed = false;

  if(t != grabbed) {
    grabbed = t;
    for(size_t i = 1; i < event_file_info.size ; ++i) {
      ioctl(event_file_info.pfds[i].fd, EVIOCGRAB, grabbed);
    }
  }
}

void write_controller(const ControllerEvent * ce)
{
  emit(uinput_file_descriptor, ce->ev_type, ce->code, ce->value);
  emit(uinput_file_descriptor, EV_SYN, SYN_REPORT, 0);
}

/* Read all available inotify events */

static void handle_inotify(bool must_grab)
{
  /* Some systems cannot read integer variables if they are not
     properly aligned. On other systems, incorrect alignment may
     decrease performance. Hence, the buffer used for reading from
     the inotify file descriptor should have the same alignment as
     struct inotify_event. */

  const char event_name[] = "event";
  const size_t event_len = strlen(event_name);
  
  char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
  
  const struct inotify_event * event;
  ssize_t                      len;
  char                       * ptr;

  /* Loop while events can be read from inotify file descriptor. */
  for (;;) {

    /* Read some events. */

    len = read(event_file_info.pfds[0].fd, buf, sizeof(buf));
    if (len == -1 && errno != EAGAIN) {
      perror("read");
      // exit(EXIT_FAILURE);
    }

    /* If the nonblocking read() found no events to read, then
       it returns -1 with errno set to EAGAIN. In that case,
       we exit the loop. */
    if (len <= 0) break;

    /* Loop over all events in the buffer */

    for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {

      event = (const struct inotify_event *) ptr;

      if(!(event->mask & IN_ISDIR) && !strncmp(event->name, event_name, event_len)) {
	if(event->mask & IN_CREATE) add_event_file(event->name,must_grab);
	if(event->mask & IN_DELETE) remove_event_file(event->name);
      }
    }
  }
}

int poll_controller(ControllerEvent * ce, int timeout)
{
  const size_t event_len = event_file_info.size;
  const int RCTRL = 0x01;
  const int RINO = 0x02;

  int p = poll(event_file_info.pfds, event_len, timeout); // Passive waiting
  if(p < 0) return -1; // error

  size_t             i = 0;
  int                quit = 0x00;
  struct input_event ie;
  
  while(p > 0 && i < event_len && !quit) {
    if(event_file_info.pfds[i].revents & POLLIN) {
      if(i == 0) {
	--p;
	handle_inotify(ce->grabbed);
	quit |= RINO; 
      }
      else {
	read(event_file_info.pfds[i].fd, &ie, sizeof(ie));
      
	if(ie.type == EV_KEY || ie.type == EV_REL || ie.type == EV_ABS) {
	  ce->controller_type = (ie.type == EV_KEY)?KEY:MOUSE;
	  ce->ev_type = ie.type;
	  ce->code = ie.code;
	  ce->value = ie.value;
	  quit |= RCTRL;
	  --p;
	}
      }
    }

    ++i;
  }
  
  return quit;
}

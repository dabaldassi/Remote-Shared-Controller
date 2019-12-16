#include <X11/Xlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>

#include "cursor.h"

static int _XlibErrorHandler(Display * display __attribute__((unused)),
			     XErrorEvent * event __attribute__((unused))) {
  fprintf(stderr, "An error occured detecting the mouse position\n");
  return True;
}

int get_cursor_info(CursorInfo * cursor)
{
  int            number_of_screens;
  int            i;
  Bool           result;
  Window       * root_windows;
  Window         window_returned;
  int            win_x, win_y;
  unsigned int   mask_return;
  Display      * display = XOpenDisplay(NULL);
  
  assert(display);
  XSetErrorHandler(_XlibErrorHandler);
  number_of_screens = XScreenCount(display);
  root_windows = malloc(sizeof(Window) * number_of_screens);
  
  for (i = 0; i < number_of_screens; i++) {
    root_windows[i] = XRootWindow(display, i);
  }
  
  for (i = 0; i < number_of_screens; i++) {
    result = XQueryPointer(display, root_windows[i], &window_returned,
			   &window_returned, &cursor->pos_x, &cursor->pos_y, &win_x, &win_y,
			   &mask_return);
    if (result == True) break;
  }
  
  if (result != True) {
    fprintf(stderr, "No mouse found.\n");
    return -1;
  }

  free(root_windows);
  XCloseDisplay(display);
  return 0;
}

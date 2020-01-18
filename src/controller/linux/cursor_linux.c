#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

#include "cursor.h"

static int _XlibErrorHandler(Display * display __attribute__((unused)),
			     XErrorEvent * event __attribute__((unused)))
{
  fprintf(stderr, "An error occured detecting the mouse position\n");
  return True;
}

CursorInfo* open_cursor_info()
{
  CursorInfo * cursor = malloc(sizeof(CursorInfo));

  XInitThreads();

  cursor->pos_x = 0;
  cursor->pos_y = 0;
  cursor->visible = true;
  cursor->display = XOpenDisplay(NULL);

  XSetErrorHandler(_XlibErrorHandler);
  assert(cursor->display);
  
  return cursor;
}

void close_cursor_info(CursorInfo * cursor)
{
  XCloseDisplay(cursor->display);
  free(cursor);
}

int get_cursor_position(CursorInfo * cursor)
{
  int            number_of_screens;
  int            i;
  Bool           result;
  Window       * root_windows;
  Window         window_returned;
  int            win_x, win_y;
  unsigned int   mask_return;
  
  number_of_screens = XScreenCount(cursor->display);
  root_windows = malloc(sizeof(Window) * number_of_screens);
  
  for (i = 0; i < number_of_screens; i++) {
    root_windows[i] = XRootWindow(cursor->display, i);
  }
  
  for (i = 0; i < number_of_screens; i++) {
    result = XQueryPointer(cursor->display, root_windows[i],
			   &window_returned,
			   &window_returned,
			   &cursor->pos_x,
			   &cursor->pos_y,
			   &win_x,
			   &win_y,
			   &mask_return);
    
    if (result == True) break;
  }
  
  if (result != True) {
    fprintf(stderr, "No mouse found.\n");
    return -1;
  }

  free(root_windows);
  return 0;
}

static void set_cursor_visibility(Display * display, bool t)
{
  Window window = XDefaultRootWindow(display);

  if(t) {
    XFixesShowCursor(display, window);
    XFlush(display);
  }
  else {
    XFixesHideCursor(display, window);
    XFlush(display);
  }
  
  XDestroyWindow(display, window);
}

void show_cursor(CursorInfo* cursor)
{
  if(!cursor->visible) {
    cursor->visible = true;
    set_cursor_visibility(cursor->display, true);
  }
}

void hide_cursor(CursorInfo* cursor)
{
  if(cursor->visible) {
    cursor->visible = false;
    set_cursor_visibility(cursor->display, false);
  }
}

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
  Display    * display = XOpenDisplay(NULL);

  XSetErrorHandler(_XlibErrorHandler);
  assert(display);
  
  CursorInfo * cursor = malloc(sizeof(CursorInfo));
  Screen     * screen = DefaultScreenOfDisplay(display);

  cursor->pos_x = 0;
  cursor->pos_y = 0;
  cursor->visible = true;
  cursor->display = display;
  cursor->screen_size.width = screen->width;
  cursor->screen_size.height = screen->height;
  
  return cursor;
}

void close_cursor_info(CursorInfo * cursor)
{
  XCloseDisplay(cursor->display);
  free(cursor);
}

int get_cursor_position(CursorInfo * cursor)
{
  Bool          result;
  Window        root_windows;
  Window        window_returned;
  int           win_x, win_y;
  unsigned int  mask_return;
  
  root_windows = XRootWindow(cursor->display, 0);
    
  result = XQueryPointer(cursor->display, root_windows,
			 &window_returned,
			 &window_returned,
			 &cursor->pos_x,
			 &cursor->pos_y,
			 &win_x,
			 &win_y,
			 &mask_return);
  
  if (result != True) {
    fprintf(stderr, "No mouse found.\n");
    return -1;
  }

  return 0;
}

int set_cursor_position(CursorInfo * cursor)
{
  Window root_window = XRootWindow(cursor->display, 0);
  
  XSelectInput(cursor->display, root_window, KeyReleaseMask);
  XWarpPointer(cursor->display, None, root_window, 0, 0, 0, 0, cursor->pos_x, cursor->pos_y);
  XFlush(cursor->display);

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

#ifndef CURSOR_H
#define CURSOR_H

#ifdef __cplusplus
extern "C" {
#endif

  #include <stdbool.h>

#if defined(__gnu_linux__)
  struct Display;
#endif
  
  typedef struct CursorInfo
  {
#if defined(__gnu_linux__)
    Display * display;
#endif
    
    int pos_x;
    int pos_y;
    bool visible;

    struct { int width; int height; } screen_size;
  }CursorInfo;

  CursorInfo * open_cursor_info();
  
  int  get_cursor_position(CursorInfo * cursor);
  int  set_cursor_position(CursorInfo * cursor);
  void show_cursor(CursorInfo * cursor);
  void hide_cursor(CursorInfo * cursor);
  void close_cursor_info(CursorInfo * cursor);

#ifdef __cplusplus
}
#endif

#endif /* CURSOR_H */

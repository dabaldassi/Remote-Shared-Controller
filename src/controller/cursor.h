#ifndef CURSOR_H
#define CURSOR_H

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct CursorInfo
  {
    int pos_x;
    int pos_y;
  }CursorInfo;

  int get_cursor_info(CursorInfo * cursor);

#ifdef __cplusplus
}
#endif

#endif /* CURSOR_H */

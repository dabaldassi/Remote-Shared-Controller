#include <stdlib.h>
#include <cursor.h>

#include <Windows.h>

CursorInfo* open_cursor_info()
{
	CursorInfo* cursor = (CursorInfo *)malloc(sizeof(CursorInfo));

	if (cursor == NULL) return NULL;

	get_cursor_position(cursor);

	CURSORINFO cursor_info;
	cursor_info.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&cursor_info);
	int x = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	
	cursor->screen_size.width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	cursor->screen_size.height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	cursor->visible = cursor_info.flags & CURSOR_SHOWING;
	
	return cursor;
}

int  get_cursor_position(CursorInfo* cursor)
{
	POINT p;
	GetCursorPos(&p);

	cursor->pos_x = p.x;
	cursor->pos_y = p.y;

	return 0;
}

int  set_cursor_position(CursorInfo* cursor)
{
	SetCursorPos(cursor->pos_x, cursor->pos_y);

	return 0;
}

void show_cursor(CursorInfo* cursor)
{
	if (!cursor->visible) {
		cursor->visible = true;
	}
}

void hide_cursor(CursorInfo* cursor)
{
	if (cursor->visible) {
		cursor->visible = false;
	}
}

void close_cursor_info(CursorInfo* cursor)
{
	free(cursor);
}
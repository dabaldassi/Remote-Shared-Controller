#include <chrono>
#include <combo.hpp>

void ComboShortcut::add_shortcut(int code, int value, int time_ms)
{
  _shortcut_list.push_back(std::make_tuple(code,value,time_ms));
  _current = _shortcut_list.end();
}

void ComboShortcut::release_for_all()
{
  std::fill_n(std::back_inserter(_shortcut_list),
	      _shortcut_list.size(),
	      std::make_tuple(int{ANY},0,-1));
}

bool ComboShortcut::update(int code, int value)
{
  using namespace std::chrono_literals;
  
  constexpr int REPEATED = 2;
  static auto last_time = std::chrono::high_resolution_clock::now();

  if(_current == _shortcut_list.end()) _current = _shortcut_list.begin();

  if(std::get<TIME>(*_current) != -1) {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = now - last_time;
    
    std::chrono::milliseconds timeout(std::get<TIME>(*_current));
    
    if(elapsed > timeout) _current = _shortcut_list.begin();

    last_time = now;
  }
  
  if(value != REPEATED) {
    if(code == std::get<CODE>(*_current) && value == std::get<VALUE>(*_current)) {
      _current++;
    }
    else if(ANY == std::get<CODE>(*_current) && value == std::get<VALUE>(*_current)) {
      _current++;
    }
    else _current = _shortcut_list.begin();
  }

  if(_current == _shortcut_list.end()) _action(this);
  
  return _current == _shortcut_list.end();
}


#ifndef NO_CURSOR
#include <cursor.h>

bool ComboMouse::update(int /* code */, int /* value */)
{
  get_cursor_position(_cursor);

  if(_cursor->pos_x == 0)               _way = Way::LEFT;
  else if(_cursor->pos_x == _width - 1) _way = Way::RIGHT;
  else                                  _way = Way::NONE;

  if(_way != Way::NONE) _action(this);
  
  return _way != Way::NONE;
}

#endif

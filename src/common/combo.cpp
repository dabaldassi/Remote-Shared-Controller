#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <combo.hpp>
#include <config.hpp>
#include <util.hpp>
#include <controller.h>

using rscutil::Combo;
using rscutil::ComboShortcut;

const std::string Combo::TYPE = "Combo";

///////////////////////////////////////////////////////////////////////////////
//                               ComboShortcut                               //
///////////////////////////////////////////////////////////////////////////////

const std::string ComboShortcut::TYPE = "ComboShortcut";

ComboShortcut::ComboShortcut(const ComboShortcut& other): Combo(Way::RIGHT)
{
  _shortcut_list = other._shortcut_list;
  _name = other._name;
  _description = other._description;
  _action = other._action;
  _current = _shortcut_list.end();
}

ComboShortcut::ComboShortcut(ComboShortcut&& other) noexcept : Combo(Way::RIGHT)
{
  _shortcut_list = std::move(other._shortcut_list);
  _name = std::move(other._name);
  _description = std::move(other._description);
  _action = std::move(other._action);
  _current = _shortcut_list.end();
}
    
ComboShortcut& ComboShortcut::operator=(const ComboShortcut& other)
{
  if(&other != this) {
    _shortcut_list = other._shortcut_list;
    _name = other._name;
    _description = other._description;
    _current = _shortcut_list.end();
    _action = other._action;
  }

  return *this;
}
    
ComboShortcut& ComboShortcut::operator=(ComboShortcut&& other) noexcept
{
  _shortcut_list = std::move(other._shortcut_list);
  _name = std::move(other._name);
  _description = std::move(other._description);
  _current = _shortcut_list.end();
  _action = std::move(other._action);

  return *this;
}

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

void ComboShortcut::for_each(const std::function<void(shortcut_t&)> &f)
{
  std::for_each(_shortcut_list.begin(), _shortcut_list.end(), f);
}

bool ComboShortcut::update(int code, int value, int, int)
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

void ComboShortcut::load(std::ifstream& ifs)
{
  size_t size;

  ifs.read((char *)&size, sizeof(size));
  _shortcut_list.clear();

  for(size_t i = 0; i < size; ++i) {
    int code, value, time;

    ifs.read((char*)&code, sizeof(int));
    ifs.read((char*)&value, sizeof(int));
    ifs.read((char*)&time, sizeof(int));

    add_shortcut(code, value, time);
  }

  rscutil::deserialize_string(ifs, _name);
  rscutil::deserialize_string(ifs, _description);
}

void ComboShortcut::save(std::ofstream &ofs) const
{
  size_t size = _shortcut_list.size();

  ofs.write((char *)&size, sizeof(size));

  for(const auto& shortcut : _shortcut_list) {
    int code, value, time;
    std::tie(code, value, time) = shortcut;

    ofs.write((char*)&code, sizeof(int));
    ofs.write((char*)&value, sizeof(int));
    ofs.write((char*)&time, sizeof(int));
  }

  rscutil::serialize_string(ofs, _name);
  rscutil::serialize_string(ofs, _description);
}

bool ComboShortcut::load(ComboShortcutList &list)
{
  std::ifstream ifs(RSC_SHORTCUT_SAVE);

  if(!ifs.is_open()) return false;

  size_t size;
  ifs.read((char *)&size, sizeof(size));
  list.clear();

  for(size_t i = 0; i < size; ++i) {
    ComboShortcut combo("", "");
    combo.load(ifs);
    list.push_back(combo);
  }
  
  ifs.close();
  return true;
}

void ComboShortcut::save(const ComboShortcutList &list)
{
  std::ofstream ofs(RSC_SHORTCUT_SAVE);

  if(ofs.is_open()) {
    size_t size = list.size();

    ofs.write((char *)&size, sizeof(size));
    
    for(const auto& shortcut : list) shortcut.save(ofs);
    ofs.close();
  }
  else throw std::runtime_error("Can't open " + std::string(RSC_SHORTCUT_SAVE));
}

std::string ComboShortcut::to_string() const
{
  std::ostringstream oss;
  size_t i = 0;

  for(const auto& a : _shortcut_list) {
    int code, value, time;
    std::tie(code, value, time) = a;

    if(value == 0)  oss << "(R)";
    
    if(code == ANY) oss << "*";
    else            oss << get_key_name_azerty(code);
    
    if(time != INFINITE) oss << "(" << time << "ms)";
    
    if(++i < _shortcut_list.size()) oss << "-";
  }

  return oss.str();
}

void ComboShortcut::make_shortcut(ComboShortcut& combo)
{
  using namespace std::chrono;
  using namespace std::chrono_literals;

  const auto timeout = 500ms;
  
  ControllerEvent ev;
  bool            end = false;
  bool            first = true;
  auto            old = high_resolution_clock::now();

  init_controller();

  // Get rid of releasing enter key from the prompt when launching.
  while(!end) {
    int ret = poll_controller(&ev, -1);
    end = (ret == 0x01) && (ev.code == KEY_S) && (ev.value == KEY_RELEASED);
  }

  write_key(KEY_BACKSPACE);
  
  grab_controller(true);
  end = false;

  while(!end) {
    int ret = poll_controller(&ev, timeout.count());
    
    if(ret == 0x01 && ev.controller_type == KEY && ev.value != KEY_REPEATED) {
      combo.add_shortcut(ev.code, ev.value, DEFAULT_TIMEOUT);
      old = high_resolution_clock::now();
      if(first) first = false;
    }

    auto now = high_resolution_clock::now();
    auto elapsed = now - old;
    if(!first && elapsed > timeout) end = true;
  }

  grab_controller(false);
  exit_controller();
}

///////////////////////////////////////////////////////////////////////////////
//                                 ComboMouse                                //
///////////////////////////////////////////////////////////////////////////////

#ifndef NO_CURSOR

using rscutil::ComboMouse;
const std::string ComboMouse::TYPE = "ComboMouse";

ComboMouse::ComboMouse(size_t width, size_t height) : _width(width),
						      _height(height)
{
}

bool ComboMouse::update(int, int, int x, int /*y*/)
{
  if(x <= 0)                       _way = Way::LEFT;
  else if((size_t)x >= _width - 1) _way = Way::RIGHT;
  else                             _way = Way::NONE;

  if(_way != Way::NONE && _action) _action(this);

  return _way != Way::NONE;
}

#endif

#ifndef COMBO_H
#define COMBO_H

#include <list>
#include <tuple>

#include <ptr.hpp>

class Combo
{
public:
  enum class Way { NONE, LEFT, RIGHT };
      
protected:
  Way _way;
 
public:
  using ptr = Ptr<Combo>::ptr;
  
  Combo(Way way = Way::NONE)
    : _way(way) {}
  virtual ~Combo() = default;

  Way get_way() { return _way; }
  virtual bool update(int code, int value) = 0;  
};

class ComboShortcut : public Combo, public Ptr<ComboShortcut>
{
  using shortcut_t = std::tuple<int,int,int>;
  
  std::list<shortcut_t>           _shortcut_list;
  std::list<shortcut_t>::iterator _current;

  enum TupleID { CODE, VALUE, TIME }; // The indice for std::get
  
public:
  using Ptr<ComboShortcut>::ptr;

  static constexpr int ANY = -1;
  static constexpr int INFINITE = -1;
  static constexpr int DEFAULT_TIMEOUT = INFINITE;
  
  ComboShortcut()
    : Combo(Way::RIGHT), _current(_shortcut_list.end()) {}
  
  void add_shortcut(int code, int value, int timeout_ms = DEFAULT_TIMEOUT);

  /**
   *\brief Add a release key for every key in the list at the end. This way, we wait until the user release all the key before changing screen. 
   */
  
  void release_for_all();
  
  virtual ~ComboShortcut() = default;
  
  bool update(int code, int value) override;
};

class ComboMouse : public Combo, public Ptr<ComboMouse>
{
  int _width, _height;
public:
  using Ptr<ComboMouse>::ptr;
  
  ComboMouse(int width, int height)
    : _width(width), _height(height) {}
  virtual ~ComboMouse() = default;
  
  bool update(int code, int value) override;
};



#endif /* COMBO_H */

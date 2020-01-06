#ifndef PC_LIST_H
#define PC_LIST_H

#include <algorithm>
#include <vector>

#include <pc.hpp>
#include <ptr.hpp>

class PCList : public Ptr<PCList>
{
  std::vector<PC> _pc_list;
  int             _current;
  bool            _circular;
  
public:  
  PCList()
    : _current{}, _circular(true) {}
  virtual ~PCList() = default;

  template<typename P>
  void add(P&& pc);

  /**
   *\ brief Add pc before the pc with id.
   */
  
  void add(const PC& pc, int id);
  void remove(int id);

  void set_circular(bool c) { _circular = c; }
  bool is_circular() { return _circular; }
  bool exist(const PC& pc) const;

  template<typename Pred>
  bool exist(Pred&& p) const;
  
  void next_pc();
  void previous_pc();
  size_t size() const { return _pc_list.size(); }
  
  void save(const std::string & file_name) const;
  void load(const std::string & file_name);
  
  const PC& get_current() const;
  const PC& get_local() const;
  const PC& get(int id) const;
};


template<typename Pred>
bool PCList::exist(Pred &&p) const
{
  auto it = std::find_if(_pc_list.begin(), _pc_list.end(), std::forward<Pred>(p));
  return it != _pc_list.end();
}

template<typename P>
void PCList::add(P&& pc) {
  auto it = std::find_if(_pc_list.begin(), _pc_list.end(), [&pc] (const PC& p) {
							     return pc.id == p.id;
							   });
  if(it == _pc_list.end()) _pc_list.push_back(std::forward<P>(pc));
  else throw std::runtime_error("id exists already");
}

#endif /* PC_LIST_H */

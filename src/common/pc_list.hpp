#ifndef PC_LIST_H
#define PC_LIST_H

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
  void add(P&& pc) { _pc_list.push_back(std::forward<P>(pc)); }

  /**
   *\ brief Add pc before the pc with id.
   */
  
  void add(const PC& pc, int id);

  void set_circular(bool c) { _circular = c; }
  bool is_circular() { return _circular; }

  void next_pc();
  void previous_pc();
  size_t size() const { return _pc_list.size(); }
  
  void save(const std::string & file_name) const;
  void load(const std::string & file_name);
  
  const PC& get_current() const;
  const PC& get_local() const;
  const PC& get(int id) const;
};


#endif /* PC_LIST_H */

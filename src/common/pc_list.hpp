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

  void set_circular(bool c) { _circular = c; }
  bool is_circular() { return _circular; }

  void next_pc();
  void previous_pc();
  void save(const std::string & file_name) const;
  
  const PC& get_current() const;
  const PC& get_local() const;
};


#endif /* PC_LIST_H */

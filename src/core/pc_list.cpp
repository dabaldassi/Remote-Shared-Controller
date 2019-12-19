#include <algorithm>
#include <pc_list.hpp>

void PCList::next_pc()
{  
  ++_current;
  
  if(_current == static_cast<int>(_pc_list.size())) {
    if(_circular) _current = 0;
    else          --_current;
  }
}

void PCList::previous_pc()
{
  --_current;

  if(_current < 0) {
    if(_circular) _current = _pc_list.size() - 1;
    else          _current = 0;
  }
}

const PC& PCList::get_current() const
{
  return _pc_list[_current];
}

const PC& PCList::get_local() const
{
  return *std::find_if(_pc_list.begin(), _pc_list.end(), [](const PC& a) {
							   return a.local;
							 });
}

void PCList::save(const std::string& // file_name
		  ) const
{
  
}

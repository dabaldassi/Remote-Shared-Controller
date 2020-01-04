#include <algorithm>
#include <fstream>

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

void PCList::add(const PC &pc, int id)
{
  auto it = std::find_if(_pc_list.begin(), _pc_list.end(), [&id](const PC& a) {
      return id == a.id;
    });

  if(it != _pc_list.end()) {
    _pc_list.insert(it, pc);
  }
  else throw std::runtime_error("PC with id " + std::to_string(id) + " not found"); 
}

void PCList::save(const std::string& file_name) const
{
  std::ofstream ofs(file_name);

  if(ofs.is_open()) {
    size_t len = _pc_list.size();
    ofs.write((char*)&len, sizeof(size_t));
    for(const PC& pc : _pc_list) pc.save(ofs);
    ofs.close();
  }
  else throw std::runtime_error("Can't open " + file_name);
}

void PCList::load(const std::string &file_name)
{
  std::ifstream ifs(file_name);
  size_t        len,i=0;

  if(ifs.is_open()) {
    _pc_list.clear();
    ifs.read((char*)&len, sizeof(size_t));
    
    while(i < len && !ifs.eof()) {
      PC pc;
      pc.load(ifs);
      _pc_list.push_back(pc);
      ++i;
    }

    ifs.close();
  }
  else throw std::runtime_error("Can't open " + file_name);
}

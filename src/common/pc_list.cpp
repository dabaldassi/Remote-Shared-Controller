#include <algorithm>
#include <fstream>

#include <pc_list.hpp>

using rscutil::PCList;
using rscutil::PC;

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

bool PCList::exist(const PC &pc) const
{
  auto it = std::find(_pc_list.begin(), _pc_list.end(), pc);

  return it != _pc_list.end();
}

void PCList::remove(int id)
{
  _pc_list.erase(std::remove_if(_pc_list.begin(),
				_pc_list.end(),
				[&id](const PC& pc) { return pc.id == id; }),
		 _pc_list.end());
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

const PC& PCList::get(int id) const
{
  auto it = std::find_if(_pc_list.begin(), _pc_list.end(), [&id](const PC& a) {
      return a.id == id;
    });

  if(it != _pc_list.end()) return *it;
  else throw std::runtime_error("PC with id " + std::to_string(id) + " not found");
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

    ofs.write((char*)&_circular, sizeof(_circular));
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
    ifs.read((char*)&_circular, sizeof(_circular));
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

void PCList::swap(int id1, int id2)
{
  auto it1 = std::find_if(_pc_list.begin(),
			  _pc_list.end(),
			  [&id1](const PC& p) { return p.id == id1; });

  auto it2 = std::find_if(_pc_list.begin(),
			  _pc_list.end(),
			  [&id2](const PC& p) { return p.id == id2; });

  if(it1 == _pc_list.end() || it2 == _pc_list.end())
    throw std::runtime_error("Id must exist");

  std::iter_swap(it1, it2);
}

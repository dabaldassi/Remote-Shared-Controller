#include <iostream>
#include <iomanip>
#include <fstream>

#include <cstring>

#include <pc.hpp>

using rscutil::PC;

void PC::save(std::ofstream &ofs) const
{
  size_t len = name.size();
  
  ofs.write((char*)&id, sizeof(id));
  ofs.write((char*)&local, sizeof(local));
  ofs.write((char*)&len, sizeof(len));
  ofs.write((char*)name.c_str(), name.size());
  ofs.write((char*)address, sizeof(*address) * LEN_ADDR);
  ofs.write((char*)&resolution, sizeof(resolution));
  ofs.write((char*)&offset, sizeof(offset));
}

void PC::load(std::ifstream& ifs)
{
  char * name_tmp;
  size_t len;
  
  ifs.read((char*)&id, sizeof(id));
  ifs.read((char*)&local, sizeof(local));
  ifs.read((char*)&len, sizeof(len));

  name_tmp = new char[len+1];
  ifs.read((char*)name_tmp, len);
  name_tmp[len] = 0;
  name = name_tmp;
  delete [] name_tmp;
  
  ifs.read((char*)address, sizeof(*address) * LEN_ADDR);
  ifs.read((char*)&resolution, sizeof(resolution));
  ifs.read((char*)&offset, sizeof(offset));
}

bool PC::operator==(const PC& other) const
{
  return id == other.id
    && local == other.local
    && name == other.name
    && resolution.h == other.resolution.h
    && resolution.w == other.resolution.w
    && offset.x == other.offset.x
    && offset.y == other.offset.y
    && !memcmp(address, other.address, sizeof(*address) * LEN_ADDR);
}

std::ostream& operator<<(std::ostream& os, const PC& pc)
{
  os << "id : " << pc.id << "\n"
     << "Name : " << pc.name << "\n"
     << "Resolution : "  << pc.resolution.w << "x" << pc.resolution.h << "\n"
     << "Offset : (x:" << pc.offset.x << "px;y:" << pc.offset.y << "px)\n";

  for(size_t i = 0; i < PC::LEN_ADDR; ++i) {
    os << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int)pc.address[i] << std::dec;
    os << ((i != PC::LEN_ADDR - 1 )? ":":"\n");
  }

  if(pc.local) os << "This pc is the local pc" << "\n";
  
  os << "\n";

  return os;
}

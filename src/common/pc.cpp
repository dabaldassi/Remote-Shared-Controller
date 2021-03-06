#include <iostream>
#include <iomanip>
#include <fstream>

#include <cstring>

#include <pc.hpp>
#include <util.hpp>

using rscutil::PC;

void PC::save(std::ofstream &ofs) const
{  
  ofs.write((char*)&id, sizeof(id));
  ofs.write((char*)&local, sizeof(local));
  ofs.write((char*)&focus, sizeof(focus));
  rscutil::serialize_string(ofs, name);
  ofs.write((char*)address, sizeof(*address) * LEN_ADDR);
  ofs.write((char*)&resolution, sizeof(resolution));
  ofs.write((char*)&offset, sizeof(offset));
}

void PC::load(std::ifstream& ifs)
{
  ifs.read((char*)&id, sizeof(id));
  ifs.read((char*)&local, sizeof(local));
  ifs.read((char*)&focus, sizeof(focus));
  rscutil::deserialize_string(ifs, name);
  ifs.read((char*)address, sizeof(*address) * LEN_ADDR);
  ifs.read((char*)&resolution, sizeof(resolution));
  ifs.read((char*)&offset, sizeof(offset));
}

bool PC::operator==(const PC& other) const
{
  return id == other.id
    && local == other.local
    && focus == other.focus
    && name == other.name
    && resolution.h == other.resolution.h
    && resolution.w == other.resolution.w
    && offset.x == other.offset.x
    && offset.y == other.offset.y
    && !memcmp(address, other.address, sizeof(*address) * LEN_ADDR);
}

std::ostream& operator<<(std::ostream& os, const PC& pc)
{
  if(pc.focus) os << "* ";
  
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

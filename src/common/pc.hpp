#ifndef PC_H
#define PC_H

#include <string>

struct PC
{
  static constexpr size_t LEN_ADDR = 6;
  
  int                      id;
  bool                     local;
  std::string              name;
  uint8_t                  address[LEN_ADDR];
  struct { int w; int h; } resolution;
  struct { int x; int y; } offset;

  void save(std::ofstream& ofs) const;
  void load(std::ifstream& ifs);

  bool operator==(const PC& other) const;
  bool operator!=(const PC& other) const { return !(*this == other); }
};

std::ostream& operator<<(std::ostream& os, const PC& pc);

#endif /* PC_H */

#ifndef PC_H
#define PC_H

#include <string>

struct PC
{
  static constexpr size_t LEN_ADDR = 6;
  
  int                      id;
  bool                     local;
  std::string              name;
  uint8_t                  adress[LEN_ADDR];
  struct { int w; int h; } resolution;
  struct { int x; int y; } offset;
};

#endif /* PC_H */

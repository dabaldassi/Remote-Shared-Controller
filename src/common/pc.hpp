#ifndef PC_H
#define PC_H

#include <string>

namespace rscutil {

  struct PC
  {
    static constexpr size_t LEN_ADDR = 6;
  
    int                      id;
    bool                     local;
    bool                     focus;
    std::string              name;
    uint8_t                  address[LEN_ADDR];
    struct { int w; int h; } resolution;
    struct { int x; int y; } offset;

    void save(std::ofstream& ofs) const;
    void load(std::ifstream& ifs);

    bool operator==(const PC& other) const;
    bool operator!=(const PC& other) const { return !(*this == other); }
  };

}  // rscutil

std::ostream& operator<<(std::ostream& os, const rscutil::PC& pc);

#endif /* PC_H */

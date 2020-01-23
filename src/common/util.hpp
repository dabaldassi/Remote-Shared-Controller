#ifndef RSC_UTIL_H
#define RSC_UTIL_H

#include <iosfwd>

namespace rscutil {

  bool is_core_running();
  void register_pid();
  void serialize_string(std::ofstream& ofs, const std::string& str);
  void deserialize_string(std::ifstream& ifs, std::string& str);

}  // rscutil

#endif /* RSC_UTIL_H */

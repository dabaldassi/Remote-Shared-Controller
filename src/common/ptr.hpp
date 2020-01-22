#ifndef PTR_H
#define PTR_H

#include <memory>

namespace rscutil {
  
  template<typename T>
  struct Ptr
  {
    using ptr = std::unique_ptr<T>;

    template<typename... Args>
    static ptr make_ptr(Args&& ... args) {
      return std::make_unique<T>(std::forward<Args>(args)...);
    }
  };

}  // rscutil

#endif /* PTR_H */

#ifndef RSCUI_H
#define RSCUI_H

#include <string>
#include <interface.h>

namespace rscutil {

  class PCList;

}  // rscutil

namespace rscui {
  
  class RscUi
  {
  public:

    virtual void display_pc(rscutil::PCList& list, bool all) = 0;
    virtual void display_if(const IF * interface) = 0;
    virtual void display_if(int if_index, const std::string& if_name) = 0;
    virtual void display_error(const std::string& error) = 0;
    virtual void display_version(const std::string& version) = 0;
    virtual void display_help() = 0;
    
    virtual ~RscUi() = default;
  };
  
}

#endif /* RSCUI_H */

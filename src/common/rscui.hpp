#ifndef RSCUI_H
#define RSCUI_H

#include <string>
#include <interface.h>

#include <combo.hpp>

namespace rscutil {

  class PCList;

}  // rscutil

namespace rscui {
  
  class RscUi
  {
  public:

    virtual void display_current_pc(rscutil::PCList& list, bool all) = 0;
    virtual void display_all_pc(rscutil::PCList& list, bool all) = 0;
    virtual void display_if(const IF * interface) = 0;
    virtual void display_if(int if_index, const std::string& if_name) = 0;
    virtual void display_error(const std::string& error) = 0;
    virtual void display_version(const std::string& version) = 0;
    virtual void display_help() = 0;
    virtual void display_shortcut(rscutil::ComboShortcut::ComboShortcutList&) = 0;
    virtual void display_shortcut(rscutil::ComboShortcut&) = 0;
    virtual void prepare_shortcut() = 0;
    virtual bool shortcut_validation(const std::string&) = 0;
    
    virtual ~RscUi() = default;
  };
  
}

#endif /* RSCUI_H */

#ifndef RSCCLI_H
#define RSCCLI_H

#include <controller_op.hpp>
#include <rscui.hpp>

namespace rscui {

  class RSCCli : public RscUi
  {
    ControllerOperation _ops;
    
  public:

    RSCCli()
      : _ops(this) {}
    
    /**
     *\brief Run the application
     *\param argc The number of arguments
     *\param argv The arguments
     */
  
    int run(int argc, char * argv[]);

    void display_pc(rscutil::PCList& list, bool all) override;
    void display_if(const IF * interface) override;
    void display_if(int if_index, const std::string& if_name) override;
    void display_error(const std::string& error) override;
    void display_version(const std::string& version) override;
    void display_help() override;
    void display_shortcut(rscutil::ComboShortcut::ComboShortcutList&) override;
    void display_shortcut(rscutil::ComboShortcut&) override;
    void prepare_shortcut() override;
    bool shortcut_validation(const std::string&) override;
  };

}  // rscui

#endif /* RSCCLI_H */

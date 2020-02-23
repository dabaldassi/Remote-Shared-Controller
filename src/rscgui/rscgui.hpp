#ifndef RSCGUI_H
#define RSCGUI_H

#include <controller_op.hpp>

#include <QMainWindow>

#include <rscui.hpp>

namespace rscui {

  class PowerPanel;
  class PCPanel;
  class Lobby;
  class ShortcutWindow;
  
  class RSCGui : public QMainWindow, public RscUi
  {
    Q_OBJECT
    
    using MenuPtr = QMenu*;
    
    ControllerOperation _ops;
    MenuPtr             _if_menu;
    PowerPanel *        _power_panel;
    Lobby      *        _lobby;
    PCPanel    *        _pc_panel;
    ShortcutWindow *    _shortcut_window;
    
  public:
    static constexpr size_t DEFAULT_WIDTH = 1024;
    static constexpr size_t DEFAULT_HEIGHT = 768;
    
    RSCGui(QWidget * parent = nullptr);

    void display_current_pc(rscutil::PCList& list, bool all) override;
    void display_all_pc(rscutil::PCList& list, bool all) override;
    void display_if(const IF * interface) override;
    void display_if(int if_index, const std::string& if_name) override;
    void display_error(const std::string& error) override;
    void display_version(const std::string& version) override;
    void display_help() override;
    void display_shortcut(rscutil::ComboShortcut::ComboShortcutList&) override;
    void display_shortcut(rscutil::ComboShortcut&) override;
    void prepare_shortcut() override;
    bool shortcut_validation(const std::string&) override;

  protected:

    /**
     *\brief Called the user request to close the window
     *\brief Close the shortcut window if opened
     */
    
    void closeEvent(QCloseEvent *event) override;
							 
  private slots:

    /**
     *\brief Called when the add button is clicked
     *\brief Add the current selection to teh current list if possible
     */
    
    void on_add();

    /**
     *\brief Called when the "add all" button is clicked
     *\brief Add all available PC to the current list
     */
    
    void on_add_all();

    /**
     *\brief Called when the timer times out
     *\brief Refresh the pc lists and check if the daemon is still here
     */
    
    void on_timeout();
    void on_pc_removed(int id);
  };

}  // rscui

#endif /* RSCGUI_H */

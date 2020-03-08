#ifndef POWER_PANEL_H
#define POWER_PANEL_H

#include <QHBoxLayout>

#include <controller_op.hpp>

class QLabel;
class QCheckBox;
class QLineEdit;

namespace rscui {

  /**
   *\brief Panel which manage the information about the daemon, 
   whether it is running, in pause or stopped
   */
  
  class PowerPanel : public QHBoxLayout
  {
    Q_OBJECT
    
  public:
    enum class State { RUNNING, PAUSED, STOPPED }; // State of the daemon
    
  private:
    State       _state;
    QLabel    * _led;
    QLabel    * _state_text;
    QLineEdit * _key;
    QCheckBox * _option_cb[ControllerOperation::NB_OPTIONS];
    uint8_t     _options;

    ControllerOperation& _ops;
    
  public:

    /**
     *\param ops A reference to the controller operation
     *\param state To initialize the panel with a state. Stopped by default
     *\param parent The parent widget.
     */
    
    explicit PowerPanel(ControllerOperation&,
			State state = State::STOPPED,
			QWidget * parent = nullptr);

    void set_option(ControllerOperation::Option opt, bool state);

    /**
     *\brief Update the state of the panel
     *\param state The new state
     */
    
    void update_state(State state);

    /**
     *\brief Get the current state.
     *\return The state
     */
    
    State get_state() const { return _state; }

  private slots:

    /**
     *\Called when the button switch is toggled
     */
    
    void on_switch_toggle(bool t);
    void on_option_clicked();
    void on_key_set();
  
  };

}  // rscui



#endif /* POWER_PANEL_H */

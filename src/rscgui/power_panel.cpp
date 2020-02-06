#include <QLabel>

#include <power_panel.hpp>
#include <config-gui.hpp>
#include <button_switch.hpp>
#include <controller_op.hpp>

using rscui::PowerPanel;

PowerPanel::PowerPanel(ControllerOperation& op, State state, QWidget * parent)
  : QHBoxLayout(parent), _ops(op)
{ 
  _led = new QLabel(parent);
  _led->setScaledContents(true);
  _led->setMaximumSize(15,15);

  _state_text = new QLabel();
  
  update_state(state);

  auto * sw = new ButtonSwitch("run", "pause",parent);
  connect(sw, &ButtonSwitch::toggle, this, &PowerPanel::on_switch_toggle);

  auto * led_layout = new QHBoxLayout;
  
  setSpacing(0);
  setContentsMargins(0,0,0,0);
  led_layout->setSpacing(10);
  led_layout->addWidget(_led, 1, Qt::AlignTop);
  led_layout->addWidget(_state_text, 1, Qt::AlignTop);
  addLayout(led_layout);
  addWidget(sw, 0, Qt::AlignTop | Qt::AlignRight);  
}

void PowerPanel::update_state(State state)
{
  QPixmap pixmap;
  QString text;
  
  _state = state;

  switch(_state) {
  case State::RUNNING:
    pixmap.load(RUNNING_LED);
    text = "Running";
    break;
  case State::PAUSED:
    pixmap.load(PAUSED_LED);
    text = "Paused";
    break;
  case State::STOPPED:
    pixmap.load(STOPPED_LED);
    text = "Stopped";
    break;
  }

  _state_text->setText(text);
  _led->setToolTip(text);
  _led->setPixmap(pixmap);
}

void PowerPanel::on_switch_toggle(bool t)
{
  if(_state != State::STOPPED) {
    if(t) {
      update_state(State::RUNNING);
      _ops.start();
    }
    else {
      update_state(State::PAUSED);
      _ops.pause();
    }
  }
}

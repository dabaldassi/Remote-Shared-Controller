#include <controller_op.hpp>

#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>

#include <power_panel.hpp>
#include <config-gui.hpp>
#include <button_switch.hpp>

using rscui::PowerPanel;
using rscui::ControllerOperation;

PowerPanel::PowerPanel(ControllerOperation& op, State state, QWidget * parent)
  : QHBoxLayout(parent), _options(0), _ops(op)
{ 
  _led = new QLabel(parent);
  _led->setScaledContents(true);
  _led->setMaximumSize(15,15);

  _state_text = new QLabel(parent);
  
  update_state(state);

  auto * sw = new ButtonSwitch("run", "pause",parent);
  connect(sw, &ButtonSwitch::toggle, this, &PowerPanel::on_switch_toggle);

  auto * led_layout = new QHBoxLayout;

  _option_cb[ControllerOperation::CIRCULAR] = new QCheckBox("circular", parent);
  for(size_t i = 0; i < ControllerOperation::NB_OPTIONS; ++i) {
    connect(_option_cb[i], &QCheckBox::clicked, this, &PowerPanel::on_option_clicked);
  }

  _key = new QLineEdit(parent);
  _key->setEchoMode(QLineEdit::Password);
  connect(_key, &QLineEdit::returnPressed, this, &PowerPanel::on_key_set);
  
  setSpacing(20);
  setContentsMargins(0,0,0,0);
  led_layout->setContentsMargins(0,0,0,0);
  led_layout->setSpacing(10);
  led_layout->addWidget(_led, 1, Qt::AlignTop);
  led_layout->addWidget(_state_text, 1, Qt::AlignTop);
  addLayout(led_layout);
  addWidget(_option_cb[ControllerOperation::CIRCULAR], 1, Qt::AlignTop | Qt::AlignHCenter);
  addWidget(_key, 1, Qt::AlignTop | Qt::AlignHCenter);
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

void PowerPanel::set_option(ControllerOperation::Option opt, bool state)
{
  if(opt >= ControllerOperation::NB_OPTIONS) return;
  
  if(state) _options |= 1u << opt;
  else      _options &= ~(1u << opt);

  _option_cb[opt]->setChecked(state);
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

void PowerPanel::on_option_clicked()
{
  // Determine which option has been clicked
  bool found = false;
  size_t i = 0;

  while(!found && i < ControllerOperation::NB_OPTIONS) {
    unsigned int state = (_option_cb[i]->checkState() == Qt::Checked) ? 1 : 0;
    if(_options ^ (state << i)) {
      _ops.set_option((ControllerOperation::Option)i, state);
      found = true;
    }
    ++i;
  }
}

void PowerPanel::on_key_set()
{
  QString key = _key->text();
  _ops.set_key(key.toStdString());
  _key->clear();
}

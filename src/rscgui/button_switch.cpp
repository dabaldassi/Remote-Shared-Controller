#include <QHBoxLayout>
#include <QPushButton>

#include <button_switch.hpp>

using rscui::ButtonSwitch;

ButtonSwitch::ButtonSwitch(QWidget * parent) : QWidget(parent)
{
  setMaximumSize(50, 50);
}

ButtonSwitch::ButtonSwitch(const QString& name1,
			   const QString& name2,
			   QWidget * parent) : QWidget(parent)
{
  QHBoxLayout * layout = new QHBoxLayout(this);

  _left_button = new QPushButton(name1, this);  
  _right_button = new QPushButton(name2, this);

  connect(_left_button,
	  &QPushButton::clicked,
	  this,
	  &ButtonSwitch::on_button_left_clicked);
  
  connect(_right_button,
	  &QPushButton::clicked,
	  this,
	  &ButtonSwitch::on_button_right_clicked);

  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(_left_button, 1);
  layout->addWidget(_right_button, 1);

  set_state(true);
}

void ButtonSwitch::on_button_right_clicked()
{
  set_state(false);

  emit toggle(false);
}

void ButtonSwitch::on_button_left_clicked()
{
  set_state(true);

  emit toggle(true);
}

void ButtonSwitch::set_state(bool t)
{
  _left_button->setDisabled(t);
  _right_button->setDisabled(!t);

  QPushButton * hide_button = (!t)? _right_button : _left_button;
  QPushButton * show_button = (t)? _right_button : _left_button;

  auto palette = hide_button->palette();
  show_button->setPalette(palette);
  show_button->setAutoFillBackground(true);
  show_button->update();
  
  palette.setColor(QPalette::Button, QColor(Qt::gray));
  palette.setColor(QPalette::ButtonText, QColor(Qt::gray));

  hide_button->setPalette(palette);
  hide_button->setAutoFillBackground(true);
  hide_button->update();
}

void ButtonSwitch::set_name(const QString& name1, const QString& name2)
{
  _left_button->setText(name1);
  _right_button->setText(name2);
}

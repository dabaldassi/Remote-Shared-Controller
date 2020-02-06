#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <shortcut_window.hpp>
#include <controller_op.hpp>

using rscui::ShortcutWindow;
using rscui::ShortcutItem;

///////////////////////////////////////////////////////////////////////////////
//                                ShortcutItem                               //
///////////////////////////////////////////////////////////////////////////////

ShortcutItem::ShortcutItem(const QString& name,
			   const QString& description,
			   const QString& shortcut,
			   QWidget * parent) : QWidget(parent), _name(name)
{
  auto * layout = new QHBoxLayout(this);
  auto * button = new QPushButton("Edit",this);

  connect(button, &QPushButton::clicked, this, &ShortcutItem::on_button_clicked);
 
  layout->addWidget(new QLabel(name, this), 1, Qt::AlignCenter);
  layout->addWidget(new QLabel(description, this), 1, Qt::AlignCenter);
  layout->addWidget(new QLabel(shortcut, this), 1, Qt::AlignCenter);
  layout->addWidget(button, 1, Qt::AlignCenter);
}

void ShortcutItem::on_button_clicked()
{
  emit shortcut_clicked(_name);
}

///////////////////////////////////////////////////////////////////////////////
//                               ShortcutWindow                              //
///////////////////////////////////////////////////////////////////////////////

ShortcutWindow::ShortcutWindow(ControllerOperation& ops, QWidget * parent) : QWidget(parent),
									     _ops(ops)
{
  _layout = new QVBoxLayout(this);
  auto * header = new QHBoxLayout();
  
  header->addWidget(new QLabel("Name", this), 1, Qt::AlignCenter);
  header->addWidget(new QLabel("Description", this), 1, Qt::AlignCenter);
  header->addWidget(new QLabel("Shortcut", this), 1, Qt::AlignCenter);
  header->addWidget(new QLabel(" ", this), 1);

  _layout->addLayout(header, 2);

  setWindowTitle("Shortcut");
}

void ShortcutWindow::add_shortcut(const QString &name,
				  const QString &description,
				  const QString &shortcut)
{
  auto * shortcut_item = new ShortcutItem(name, description, shortcut, this);
  connect(shortcut_item, &ShortcutItem::shortcut_clicked, this, &ShortcutWindow::on_setshortcut);
  _layout->addWidget(shortcut_item, 1);
}

void ShortcutWindow::on_setshortcut(const QString &name)
{
  _ops.set_shortcut(name.toStdString());
}

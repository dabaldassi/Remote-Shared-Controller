#include <QApplication>

#include <menu_bar.hpp>
#include <controller_op.hpp>
#include <config-gui.hpp>

using namespace rscui;

///////////////////////////////////////////////////////////////////////////////
//                                  FileMenu                                 //
///////////////////////////////////////////////////////////////////////////////

FileMenu::FileMenu(ControllerOperation& ops, QWidget * parent) : QMenu("&File", parent),
								 _ops(ops)
{
  QPixmap quitpix(QUIT_ICON);
  auto * quit = new QAction("&Quit", this);
   
  quit->setIcon(QIcon(quitpix));
  quit->setShortcut(tr(QUIT_SHORTCUT));
  
  addSeparator();
  addAction(quit);

  connect(quit, &QAction::triggered, qApp, &QApplication::quit);
}

///////////////////////////////////////////////////////////////////////////////
//                                   IfMenu                                  //
///////////////////////////////////////////////////////////////////////////////

IfCheckbox::IfCheckbox(int id, const QString& name, QWidget * parent)
  : QAction(parent), _id(id)
{
  QString  msg = QString::number(id) + ". " + name;

  setCheckable(true);
  setChecked(false);
  setText(msg);

  connect(this, &QAction::triggered, this, &IfCheckbox::on_check);
}

void IfCheckbox::on_check()
{
  emit checked(_id);
}

IfMenu::IfMenu(ControllerOperation& ops, QWidget * parent) :
  QMenu("&Interface", parent), _ops(ops)
{
  connect(this, &QMenu::aboutToShow, this, &IfMenu::refresh_interface);
}

void IfMenu::set_interface(const QList<QPair<int, QString>>& list)
{
  for(const auto& i : _interfaces) removeAction(i);
  _interfaces.clear();
  
  for(const auto& i : list) {
    Checkbox interface = new IfCheckbox(i.first, i.second, this);

    addAction(interface);

    connect(interface, &IfCheckbox::checked, this, &IfMenu::setif);
    
    _interfaces.insert(i.first, interface);
  }
}

void IfMenu::set_interface(int index)
{
  _interfaces[index]->setChecked(true);
}

void IfMenu::refresh_interface()
{
  _ops.listif();
}

void IfMenu::setif(int index)
{
  _ops.setif(std::to_string(index));
}

///////////////////////////////////////////////////////////////////////////////
//                                ShortcutMenu                               //
///////////////////////////////////////////////////////////////////////////////

ShortcutMenu::ShortcutMenu(ControllerOperation& ops, QWidget * parent)
  : QMenu("&Shortcut",parent),
    _ops(ops)
{
  auto * list_shortcut = new QAction("&List", this);
  auto * reset_shortcut = new QAction("&Reset", this);

  connect(list_shortcut, &QAction::triggered, this, &ShortcutMenu::on_list);
  connect(list_shortcut, &QAction::triggered, this, &ShortcutMenu::on_reset);
  
  addAction(list_shortcut);
  addSeparator();
  addAction(reset_shortcut);
}

void ShortcutMenu::on_list()
{
  _ops.list_shortcut();
}

void ShortcutMenu::on_reset()
{
  _ops.reset_shortcut();
}


///////////////////////////////////////////////////////////////////////////////
//                                  HelpMenu                                 //
///////////////////////////////////////////////////////////////////////////////

HelpMenu::HelpMenu(ControllerOperation& ops, QWidget * parent) : QMenu("&Help", parent),
								 _ops(ops)
{
  auto * version = new QAction("&Version", this);
  auto * help = new QAction("&Help", this);

  help->setShortcut(tr(HELP_SHORTCUT));
  
  addAction(help);
  addAction(version);
  
  connect(version, &QAction::triggered, this, &HelpMenu::version);
  connect(help, &QAction::triggered, this, &HelpMenu::help);
}

void HelpMenu::version()
{
  _ops.version();
}

void HelpMenu::help()
{
  _ops.help();
}

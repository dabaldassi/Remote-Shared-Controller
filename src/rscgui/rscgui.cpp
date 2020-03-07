#include <iomanip>

#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>

#include <rscgui.hpp>
#include <menu_bar.hpp>
#include <power_panel.hpp>
#include <lobby.hpp>
#include <pc_panel.hpp>
#include <shortcut_window.hpp>
#include <config-gui.hpp>

#include <pc_list.hpp>
#include <util.hpp>

using rscui::RSCGui;

RSCGui::RSCGui(QWidget * parent) : QMainWindow(parent),
				   _ops(this),
				   _shortcut_window(nullptr)
{
  resize(DEFAULT_WIDTH,DEFAULT_HEIGHT);
  setWindowIcon(QIcon(LOGO_RSC));
  
  _if_menu = new IfMenu(_ops, this);
  
  menuBar()->addMenu(new FileMenu(_ops,this));
  menuBar()->addMenu(_if_menu);
  menuBar()->addMenu(new ShortcutMenu(_ops,this));
  menuBar()->addMenu(new HelpMenu(_ops,this));

  PowerPanel::State state;
  
  state = (rscutil::is_core_running()) ? PowerPanel::State::RUNNING : PowerPanel::State::STOPPED;
  
  _power_panel = new PowerPanel(_ops,state,this);
  
  QVBoxLayout * layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->addLayout(_power_panel, 1); // PowerPanel deleted by layout

  auto * hbox = new QHBoxLayout;

  _pc_panel = new PCPanel(_ops, this);
  connect(_pc_panel, &PCPanel::pc_removed, this, &RSCGui::on_pc_removed);
  hbox->addLayout(_pc_panel, 7);

  auto * button_panel = new QVBoxLayout();
  auto * button = new QPushButton("<", this);

  connect(button, &QPushButton::clicked, this, &RSCGui::on_add);
  button_panel->addWidget(button);

  button = new QPushButton("<<", this);
  connect(button, &QPushButton::clicked, this, &RSCGui::on_add_all);
  button_panel->addWidget(button);

  hbox->addLayout(button_panel, 1);
  
  _lobby = new Lobby(_ops, this);
  hbox->addLayout(_lobby, 2);

  layout->addLayout(hbox, 4);

  QWidget * window = new QWidget(this);
  window->setLayout(layout);
  setCentralWidget(window);

  auto * timer = new QTimer(this);
  timer->setInterval(5000);
  timer->start();

  connect(timer, &QTimer::timeout, this, &RSCGui::on_timeout);

  auto * pcwidget = new PCWidget(0, "localhost", nullptr);
  pcwidget->focus(true);
  _pc_panel->add_pc(pcwidget);
  
  on_timeout();
}

void RSCGui::display_current_pc(rscutil::PCList& list, bool)
{
  QLayoutItem * child;

  _power_panel->set_option(ControllerOperation::Option::CIRCULAR, list.is_circular());
  
  if(PCWidget::can_update) {
  
    while((child = _pc_panel->takeAt(0))) {
      delete child->widget();
      delete child;
    }
  
    for(size_t i = 0; i < list.size(); ++i) {
      const rscutil::PC& pc = list.get_current();
      auto * pcwidget = new PCWidget(pc.id, pc.name.c_str(), nullptr);

      pcwidget->focus(pc.focus);

      auto * pc_lobby = _lobby->get(pc.name.c_str());

      if(pc_lobby) pc_lobby->set_used(true);
      
      _pc_panel->add_pc(pcwidget);
      list.next_pc();      
    }
  }
}

void RSCGui::display_all_pc(rscutil::PCList& list, bool)
{
  _lobby->clear_pc();
  
  for(size_t i = 0; i < list.size(); ++i) {
    auto * pc_item = new PCWidgetItem(nullptr);
    const rscutil::PC& pc = list.get_current();
    
    pc_item->set_name(pc.name.c_str());
    pc_item->set_id(pc.id);

    std::ostringstream oss;

    for(size_t i = 0; i < rscutil::PC::LEN_ADDR; ++i) {
      oss << std::setw(2) << std::setfill('0') << std::hex << int{pc.address[i]} << ":";
    }

    pc_item->set_address(oss.str().c_str());

    _lobby->add_pc(pc_item);
    
    list.next_pc();
  }
}


void RSCGui::display_if(const IF * interface)
{
  QList<QPair<int, QString>> list;
  
  for(const IF * i = interface; !(i->if_index == 0 && i->if_name == nullptr); ++i) {
    list.append(QPair<int,QString>(i->if_index, i->if_name));
  }

  static_cast<IfMenu*>(_if_menu)->set_interface(list);

  if(rscutil::is_core_running()) _ops.getif();
}

void RSCGui::display_if(int index, const std::string&)
{
  static_cast<IfMenu*>(_if_menu)->set_interface(index);
}

void RSCGui::display_error(const std::string& err)
{
  QMessageBox::critical(this, tr("Error"), tr(err.c_str()));
}

void RSCGui::display_version(const std::string& version)
{
  QMessageBox::about( 
    this, 
    tr("Remote Shared Controller "), 
    tr(("Version " + version).c_str()));
}

void RSCGui::display_help()
{
  QMessageBox::information(this, "Help", "There is no help");
}

void RSCGui::display_shortcut(rscutil::ComboShortcut::ComboShortcutList& list)
{
  if(_shortcut_window) delete _shortcut_window;
  
  _shortcut_window = new ShortcutWindow(_ops);
  _shortcut_window->show();
  
  connect(this, &QObject::destroyed, _shortcut_window, &QWidget::close);

  for(const auto& s : list) {
    _shortcut_window->add_shortcut(s.get_name().c_str(),
				   s.get_description().c_str(),
				   s.to_string().c_str());
  }
}

void RSCGui::display_shortcut(rscutil::ComboShortcut&)
{
  
}

void RSCGui::prepare_shortcut()
{
  _shortcut_window->close();
  QMessageBox::information(this, tr("Set shortcut"), tr("Press 's' then type the new shortcut"));
}

bool RSCGui::shortcut_validation(const std::string& str)
{
  int ret = QMessageBox::question(this, "Shortcut validation", str.c_str());
  
  return ret == QMessageBox::Yes;
}

void RSCGui::on_add()
{
  auto * selected_pc = _lobby->get_selected();

  if(selected_pc && !selected_pc->get_used()) {
    auto * pc = new PCWidget(selected_pc->get_id(), selected_pc->get_name(), nullptr);
    _pc_panel->add_pc(pc);
    _ops.add(std::to_string(selected_pc->get_id()));
    selected_pc->set_used(true);
  }
}

void RSCGui::on_add_all()
{
  for(int i = 0; i < _lobby->count_pc(); ++i) {
    auto * selected_pc = (*_lobby)[i];
    if(selected_pc->get_used()) continue;
    
    auto * pc = new PCWidget(selected_pc->get_id(), selected_pc->get_name(), nullptr);
    _pc_panel->add_pc(pc);
    _ops.add(std::to_string(selected_pc->get_id()));
    selected_pc->set_used(true);
  }
}

void RSCGui::on_timeout()
{
  bool running = rscutil::is_core_running();
  if(running) {
    if(_power_panel->get_state() == PowerPanel::State::STOPPED) {
      _power_panel->update_state(PowerPanel::State::RUNNING);
    }

    if(_power_panel->get_state() == PowerPanel::State::RUNNING) {
      _ops.listrefresh(true);
      _ops.listcurrent(true);
    }
  }
  else {
    if(_power_panel->get_state() != PowerPanel::State::STOPPED) {
      _power_panel->update_state(PowerPanel::State::STOPPED);
    }
  }
}

void RSCGui::closeEvent(QCloseEvent *)
{
  if(_shortcut_window) {
    _shortcut_window->close();
    delete _shortcut_window;
    _shortcut_window = nullptr;
  }
}

void RSCGui::on_pc_removed(int id)
{
  for(int i = 0; i < _lobby->count_pc(); ++i) {
    auto * pc = (*_lobby)[i];
    if(pc->get_id() == id) pc->set_used(false);
  }
}

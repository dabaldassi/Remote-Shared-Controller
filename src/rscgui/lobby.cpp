#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <controller_op.hpp>

#include <QTextStream>

#include <lobby.hpp>

using rscui::PCWidgetItem;
using rscui::Lobby;
using rscui::ControllerOperation;

///////////////////////////////////////////////////////////////////////////////
//                                PCWidgetItem                               //
///////////////////////////////////////////////////////////////////////////////

PCWidgetItem::PCWidgetItem(QListWidget * parent) : QListWidgetItem(parent)
{
  set_used(false);
}

void PCWidgetItem::print_tooltip()
{
  QString     tooltip;
  QTextStream stream(&tooltip);

  stream << "id : " << _id << "\n"
	 << "name : " << _name << "\n"
	 << "address : " << _address << "\n";
  
  setToolTip(*stream.string());
}

PCWidgetItem::PCWidgetItem(const QString& name, QListWidget * parent)
  : QListWidgetItem(name, parent)
{
}

void PCWidgetItem::set_used(bool t)
{
  _used = t;
 
  if(t) setCheckState(Qt::CheckState::Checked);
  else  setCheckState(Qt::CheckState::Unchecked);
}

void PCWidgetItem::set_id(int id)
{  
  _id = id; 
}

void PCWidgetItem::set_name(const QString& name)
{
  _name = name;
  setText(name);
}

void PCWidgetItem::set_address(const QString& address)
{
  _address = address;
}

///////////////////////////////////////////////////////////////////////////////
//                                   Lobby                                   //
///////////////////////////////////////////////////////////////////////////////

Lobby::Lobby(ControllerOperation& op, QWidget * parent) : QVBoxLayout(parent),
							  _ops(op),
							  _selection(nullptr)
{
  auto * header = new QHBoxLayout();
  auto * refresh = new QPushButton(parent);

  connect(refresh, &QPushButton::clicked, this, &Lobby::on_refresh_clicked);
  
  header->addWidget(new QLabel("PC",parent), 3, Qt::AlignLeft | Qt::AlignVCenter);
  header->addWidget(refresh, 1, Qt::AlignLeft | Qt::AlignVCenter);
  addLayout(header, 1);

  _pc_list = new QListWidget(parent);
  connect(_pc_list, &QListWidget::itemClicked, this, &Lobby::on_item_activated);

  addWidget(_pc_list, 8);
}

int Lobby::add_pc(PCWidgetItem * pc)
{  
  _pc_list->addItem(pc);
  pc->print_tooltip();
  return 0;
}

void Lobby::clear_pc()
{
  _pc_list->clear();
  _selection = nullptr;
}

void Lobby::on_refresh_clicked()
{
  _ops.listrefresh(true);
  _ops.listcurrent(true);
  _selection = nullptr;
}

void Lobby::on_item_activated(QListWidgetItem * item)
{
  _selection = static_cast<PCWidgetItem *>(item);
}

PCWidgetItem * Lobby::get(const QString &name)
{
  auto res = _pc_list->findItems(name, Qt::MatchExactly);

  if(res.empty()) return nullptr;

  return static_cast<PCWidgetItem *>(res.front());
}

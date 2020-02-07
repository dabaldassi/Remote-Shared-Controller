#include <QLabel>
#include <QtGui>
#include <QDrag>
#include <QApplication>
#include <QMenu>

#include <pc_panel.hpp>
#include <controller_op.hpp>

using rscui::PCWidget;
using rscui::PCPanel;

///////////////////////////////////////////////////////////////////////////////
//                                  PCWidget                                 //
///////////////////////////////////////////////////////////////////////////////

const QColor PCWidget::_selected_color{255,0,0};
const QColor PCWidget::_base_color{0,0,255};

bool PCWidget::can_update = true;

PCWidget::PCWidget(int id, const QString& name, QWidget * parent) : QWidget(parent),
								    _focus(false),
								    _id(id)
{
  setAcceptDrops(true);
  
  auto * hbox = new QHBoxLayout(this);
  _name = new QLabel(name);
  hbox->addWidget(_name, 1, Qt::AlignCenter);

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &QWidget::customContextMenuRequested, this, &PCWidget::on_contextmenu);
}

void PCWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        _dragStartPosition = event->pos();
}

void PCWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & Qt::LeftButton)) return;
    if((event->pos() - _dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    QDrag     * drag = new QDrag(this);
    QMimeData * mimeData = new QMimeData;

    mimeData->setData(PCWidget::PCMimeType(), nullptr);
    drag->setMimeData(mimeData);

    can_update = false;
    drag->exec();
    can_update = true;
}

void PCWidget::paintEvent(QPaintEvent* e)
{
  QPainter qp(this);
  QPoint   p;

  p.setX(_name->pos().x() - _name->width() / 2);
  p.setY(_name->pos().y() + _name->height() - _name->width());

  if(focus()) qp.setPen(_selected_color);
  else        qp.setPen(_base_color);
    
  qp.drawRect(p.x(),p.y(),_name->width() * 2, _name->width() * 2);

  QWidget::paintEvent(e);
}

void PCWidget::swap(PCWidget* other)
{
  bool    focus = other->_focus;
  int     id = other->_id;
  QString name = other->_name->text();

  other->_focus = _focus;
  other->_id = _id;
  other->_name->setText(_name->text());

  _focus = focus;
  _id = id;
  _name->setText(name);
}

void PCWidget::dragEnterEvent(QDragEnterEvent *event)
{
  if(event->mimeData()->hasFormat(PCWidget::PCMimeType())) {
    event->acceptProposedAction();
  }
}

void PCWidget::dropEvent(QDropEvent *event)
{
  if(event->source() == this) return;

  if(event->mimeData()->hasFormat(PCWidget::PCMimeType())) {
    event->acceptProposedAction();
    auto * src = static_cast<PCWidget *>(event->source());   
    swap(src);

    emit pc_swapped(_id, src->_id);
  }
}

void PCWidget::on_contextmenu(const QPoint& pos)
{
  QMenu * contextMenu = new QMenu(tr("Context menu"), this);

  QAction * action1 = new QAction("Remove PC", this);

  if(_name->text() == "localhost") action1->setDisabled(true);
  
  connect(action1, &QAction::triggered, this, &PCWidget::on_remove);
  contextMenu->addAction(action1);

  contextMenu->exec(mapToGlobal(pos));
}

void PCWidget::on_remove()
{
  emit pc_removed(_id);
  deleteLater();
}

///////////////////////////////////////////////////////////////////////////////
//                                  PCPanel                                  //
///////////////////////////////////////////////////////////////////////////////

PCPanel::PCPanel(ControllerOperation& op, QWidget * parent) : QHBoxLayout(parent), _ops(op)
{
}

void PCPanel::add_pc(PCWidget *pc)
{
  addWidget(pc);
  connect(pc, &PCWidget::pc_swapped, this, &PCPanel::on_swapped);
  connect(pc, &PCWidget::pc_removed, this, &PCPanel::on_remove);
}

void PCPanel::on_remove(int id)
{
  _ops.remove(std::to_string(id));
  emit pc_removed(id);
}

void PCPanel::on_swapped(int id1, int id2)
{
  _ops.swap(id1, id2);
}

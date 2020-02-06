#ifndef PC_PANEL_H
#define PC_PANEL_H

#include <QHBoxLayout>
#include <QWidget>

class QLabel;

namespace rscui {

  class ControllerOperation;
  
  class PCWidget : public QWidget
  {
    bool     _focus;
    int      _id;
    QLabel * _name;
    QPoint   _dragStartPosition;
    
    static const QColor _selected_color;
    static const QColor _base_color;

    Q_OBJECT
    
  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    
    void paintEvent(QPaintEvent* e) override;
    
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  public:
    // The item can't be updated / deleted while an item is dragged
    static bool can_update;
    
    static QString PCMimeType() { return QStringLiteral("text/pc"); }
    
    explicit PCWidget(int id, const QString& name, QWidget * parent = nullptr);

    void focus(bool t) { _focus = t; }
    bool focus() const { return _focus; }

    int id() const { return _id; }

    /**
     *\brief Swap the PCWidget with another PCWidget
     *\param other The other PC
     */
    
    void swap(PCWidget* other);

  signals:

    /**
     *\brief This signal is sent when a swap occured
     *\param id1 The id of the first PC
     *\param id2 The id of the second PC
     */
    
    void pc_swapped(int id1,int id2);

    /**
     *\brief This signal is sent when a remove is requested.
     *\param id The id of the PC to remove
     */
    
    void pc_removed(int id);

  private slots:

    /**
     *\brief Called a the user right-clicks on the PCWidget
     *\param pos The position of the cursor
     */
    
    void on_contextmenu(const QPoint & pos);

    /**
     *\brief Called when the remove item from the context menu is called
     */
    
    void on_remove();
  };


  class PCPanel : public QHBoxLayout
  {
    ControllerOperation& _ops;

    Q_OBJECT

  public:
    explicit PCPanel(ControllerOperation&, QWidget * parent = nullptr);

    /**
     *\brief Add a PCWidget to the panel
     *\param pc The PCWidget to add
     */
    
    void add_pc(PCWidget * pc);

  private slots:
    
    /**
     *\brief Called when a remove is requested
     *\brief Remove a PC from the current list
     *\param id The id of the PCWidget to remove 
     */
    
    void on_remove(int id);

    /**
     *\brief Called when two PC are swapped
     *\brief Swapped the two PC in the current list
     *\param id1 The id of the first PC
     *\param id2 The id of the second PC
     */
    
    void on_swapped(int id1, int id2);
  };


}  // rscui


#endif /* PC_PANEL_H */

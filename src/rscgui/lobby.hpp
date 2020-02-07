#ifndef LOBBY_H
#define LOBBY_H

#include <QVBoxLayout>
#include <QListWidget>

namespace rscui {

  class ControllerOperation;

  /**
   *\brief Handle the display of a PC
   */
  
  class PCWidgetItem : public QListWidgetItem
  {    
    int     _id;
    QString _name;
    QString _address;
    bool    _used;
    
  public:

    /**
     *\param parent The QListwidget which will add this item
     */
    
    explicit PCWidgetItem(QListWidget * parent = nullptr);

    /**
     *\param name The PC hostname
     *\param parent The QListwidget which will add this item
     */
    
    explicit PCWidgetItem(const QString& name, QListWidget * parent = nullptr);

    /**
     *\brief Set if the PC is currently used in the list.
     *\param t True if used. False is unused.
     */
    
    void set_used(bool t);

    /**
     *\brief Set the PC id.
     *\param id The id
     */
    
    void set_id(int id);

    /**
     *\brief Set the PC hostname
     *\param name The hostname
     */
    
    void set_name(const QString& name);

    /**
     *\brief Set the PC MAC address
     *\param The address as a QString
     */
    
    void set_address(const QString& address);

    /**
     *\brief Set the tooltip of the item. The tootip contains the ID, the name and the address
     */
    
    void print_tooltip();

    /**
     *\brief Tell whether the PC is used or not.
     *\return true is used. False if unused
     */
    
    bool get_used() const { return _used; }

    /**
     *\brief Get the PC id
     *\return The id.
     */
    
    int get_id() const { return _id; }

    /**
     *\brief Get the PC hostname
     *\return The hostname
     */
    
    const QString& get_name() const { return _name; }

    /**
     *\brief Get the PC mac address
     *\return The address as a QString
     */
    
    const QString& get_address() const { return _address; }    
  };

  /**
   *\brief List of all PC running the daemon in the local network
   */
  
  class Lobby : public QVBoxLayout
  {
    Q_OBJECT

    ControllerOperation& _ops;
    QListWidget  *       _pc_list;
    PCWidgetItem *       _selection;
    
  public:

    /**
     *\param ops The rsc controller operation.
     *\param parent The QWidget parent
     */
    
    explicit Lobby(ControllerOperation&, QWidget * parent = nullptr);

    /**
     *\brief Add a PC to the lobby
     *\param pc The new PC
     */
    
    int add_pc(PCWidgetItem* pc);

    /**
     *\brief Get the current selected PC
     *\return The pc
     */
    
    const PCWidgetItem * get_selected() const { return _selection; }
    PCWidgetItem * get_selected() { return _selection; }

    /**
     *\brief Get the number of PC in the lobby
     */
    
    int count_pc() const { return _pc_list->count(); }


    PCWidgetItem * get(const QString& name);

    const PCWidgetItem * operator[](unsigned ind) const {
      return static_cast<PCWidgetItem *>(_pc_list->item(ind));
    }

    PCWidgetItem * operator[](unsigned ind) {
      return static_cast<PCWidgetItem *>(_pc_list->item(ind));
    }

    /**
     *\brief Clear the lobby by removing all PC.
     */
    
    void clear_pc();

  private slots:
    
    /**
     *\brief Called when the refresh button is clicked
     */
    
    void on_refresh_clicked();

    /**
     *\brief Called when a PC in the lobby is clicked
     *\param item The PC item
     */
    
    void on_item_activated(QListWidgetItem * item);
  };


}  // rscui

#endif /* LOBBY_H */

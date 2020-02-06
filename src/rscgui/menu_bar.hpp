#ifndef MENU_BAR_H
#define MENU_BAR_H

#include <QMenu>
#include <QAction>

namespace rscui {

  class ControllerOperation;
  
  class FileMenu : public QMenu
  {
    ControllerOperation& _ops;
    
  public:
    FileMenu(ControllerOperation&, QWidget * parent = nullptr);
  };

  class IfCheckbox : public QAction
  {
    int     _id;

    Q_OBJECT
    
  public:
    IfCheckbox(int id, const QString& name,  QWidget * parent = nullptr);

  private slots:

    /**
     *\biref Called when the checkbox is triggered
     */
    
    void on_check();

  signals:

    /**
     *\brief This signal is emitted when the checkbox is triggered
     *\param id The id of the checkbox
     */
    
    void checked(int id);
  };


  class IfMenu : public QMenu
  {
    using Checkbox = IfCheckbox*;
    
    ControllerOperation& _ops;
    QMap<int, Checkbox>  _interfaces;

    Q_OBJECT
    
  public:
    IfMenu(ControllerOperation&, QWidget * parent = nullptr);

    /**
     *\brief Display all the network interfaces
     *\param list The list of all the interface. Each element is a pair of the index and the name.
     */
    
    void set_interface(const QList<QPair<int, QString>>& list);

    /**
     *\brief Set the interface used by the daemon
     *\warning Must be call after setting all the network interfaces
     *\pram index The index of the used interface
     */
    
    void set_interface(int index);

  private slots:

    /**
     *\brief Called when the menu is clicked
     *\breif Refresh the network interface
     */
    
    void refresh_interface();

    /**
     *\brief Called when an interface in the list is clicked.
     *\param id The interface index
     */
    
    void setif(int id);
  };

  class ShortcutMenu : public QMenu
  {
    ControllerOperation& _ops;

    Q_OBJECT
    
  public:
    ShortcutMenu(ControllerOperation&, QWidget * parent = nullptr);

  private slots:

    /**
     *\brief Called when the item list is clicked
     *\brief List all the rsc shortcuts
     */
    
    void on_list();

    /**
     *\brief Called when the item reset is clicked
     *\brief Reset the default rsc shortcuts
     */
    
    void on_reset();
  };

  class HelpMenu : public QMenu
  {
    ControllerOperation& _ops;

    Q_OBJECT
    
  public:
    HelpMenu(ControllerOperation&, QWidget * parent = nullptr);

  private slots:

    /**
     *\brief Called when the item version is clicked
     *\brief Get the current version of rsc
     */
    
    void version();

    /**
     *\brief Called when the item help is clicked
     *\brief Show the help
     */
    
    void help();
  };


}  // rscutil

#endif /* MENU_BAR_H */

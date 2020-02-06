#ifndef SHORTCUT_WINDOW_H
#define SHORTCUT_WINDOW_H

#include <QWidget>

class QVBoxLayout;

namespace rscui {

  class ControllerOperation;

  class ShortcutItem : public QWidget
  {
    QString _name;
    
    Q_OBJECT
  public:

    explicit ShortcutItem(const QString& name,
			  const QString& description,
			  const QString& shortcut,
			  QWidget * parent = nullptr);

  signals:
    void shortcut_clicked(const QString& name);
      
  private slots:
    void on_button_clicked();
    
  };
  
  class ShortcutWindow : public QWidget
  {
    ControllerOperation& _ops;
    QVBoxLayout *        _layout;

    Q_OBJECT
  public:
    explicit ShortcutWindow(ControllerOperation& ops, QWidget * parent = nullptr);

    /**
     *\brief add a shortcut to the list
     *\param name The name of the shortcut
     *\param description THe description of the shortcut
     *\param shortcut The shortcut as a QString
     */
    
    void add_shortcut(const QString& name, const QString& description, const QString& shortcut);

  private slots:

    /**
     *\brief Called when the button edit corresponding to the shortcut is clicked
     *\param name The name of the shortcut
     */
    
    void on_setshortcut(const QString& name);
  };


}  // rsgui

#endif /* SHORTCUT_WINDOW_H */

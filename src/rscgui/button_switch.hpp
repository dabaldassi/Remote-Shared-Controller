#ifndef BUTTON_SWITCH_H
#define BUTTON_SWITCH_H

#include <QWidget>

class QPushButton;

namespace rscui {

  class ButtonSwitch : public QWidget
  {
    QPushButton * _left_button, * _right_button;
    Q_OBJECT
    
  public:

    /**
     *\param parent The QWidget parent
     */
    
    explicit ButtonSwitch(QWidget * parent = nullptr);

    /**
     *\param name1 The name on the left of the switch
     *\param name2 The name on the right of the switch
     *\param parent The QWidget parent
     */
    
    explicit ButtonSwitch(const QString& name1,
			  const QString& name2,
			  QWidget * parent = nullptr);

    /**
     *\brief Set the name of the switch.
     *\param name1 The name on the left of the switch
     *\param name2 The name on the right of the switch
     */
    
    void set_name(const QString& name1, const QString& name2);

    /**
     *\brief Set the state of the switch
     *\param t Left button is pressed if true. Right button is pressed otherwise
     */
    
    void set_state(bool t);
									  
  private slots:

    /**
     *\brief Called when the right button is clicked
     */
    
    void on_button_right_clicked();

    /**
     *\brief Called when the left button is clicked
     */
    
    void on_button_left_clicked();    
  signals:

    /**
     *\param This signal is emittied when a button is clicked. It means the state of the switch is toggled.
     *\param t True if left button is clicked, false if right button is clicked.
     */

    void toggle(bool t);
  };


}  // rscui

#endif /* BUTTON_SWITCH_H */

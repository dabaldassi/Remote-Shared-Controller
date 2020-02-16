#ifndef RSCLOCAL_COM_H
#define RSCLOCAL_COM_H

#include <sstream>

#include <message.hpp>

#if defined(__gnu_linux__)
#include <linux/fifo.hpp>

namespace rsclocalcom {

  using IPC = Fifo;
  
}  // rsclocalcom

#else

#include <windows/fifo.hpp>

namespace rsclocalcom {
    using IPC = Fifo;
}

#endif

namespace rsclocalcom {
  
  template<typename T>
  class RSCLocalComImpl
  {
    T _com_impl;
  public:
    using Contact = typename T::Contact;
  
    explicit RSCLocalComImpl(Contact c)
      : _com_impl(c) { _com_impl.open(); }

    /**
     *\brief Send a message to specified contact
     *\param c The contact who will receive the message
     *\param msg The message to send
     */
    
    void send_to(Contact c, const Message& msg);

    /**
     *\brief Receive a message from a contact.
     *\param c The contact who send the message
     *\param buffer The buffer in which the message will be stored
     */
    
    void read_from(Contact c, Message& buffer);

    /**
     *\brief Send a message to the other pair
     *\param msg The message to send
     */
    
    void send(const Message& msg);

    /**
     *\brief Receive a message from the other pair..
     *\param buffer The buffer in which the message will be stored
     */
    
    void read(Message& buffer);
  
    ~RSCLocalComImpl() { _com_impl.close(); }
  };

  template<typename T>
  void RSCLocalComImpl<T>::send_to(Contact c, const Message& msg)
  {
    std::stringstream ss;
    
    msg.get(ss);
    _com_impl.send_to(c, ss.str());
  }

  template<typename T>
  void RSCLocalComImpl<T>::read_from(Contact c, Message& msg)
  {
    std::stringstream ss;
    std::string       buffer;
    
    _com_impl.read_from(c, buffer);

    ss.str(buffer);
    msg.set(ss);
  }

  template<typename T>
  void RSCLocalComImpl<T>::send(const Message& msg)
  {
    std::stringstream ss;
    
    msg.get(ss);
    _com_impl.send(ss.str());
  }

  template<typename T>
  void RSCLocalComImpl<T>::read(Message& msg)
  {
    std::stringstream ss;
    std::string       buffer;
    
    _com_impl.read(buffer);

    ss.str(buffer);
    msg.set(ss);
  }

  /**
   *\class RSClocalcom
   *\brief The aim of this class is to set a communication between the core and the
   * interface through an intern process communication. This class is a wrapper of
   * an IPC specific to the operating system.
   */
  
  using RSCLocalCom = RSCLocalComImpl<IPC>;

}  // rsclocalcom

#endif /* RSCLOCAL_COM_H */

#ifndef RSCLOCAL_COM_H
#define RSCLOCAL_COM_H

#include <sstream>

#include <message.hpp>

#if defined(__gnu_linux__)
#include <linux/fifo.hpp>

namespace rsclocalcom {

  using IPC = Fifo;
  
}  // rsclocalcom

#endif

namespace rsclocalcom {
  
  template<typename T>
  class RSCLocalComImpl
  {
    T _com_impl;
  public:
    using Contact = typename T::Contact;
  
    RSCLocalComImpl() { _com_impl.open(); }

    void send_to(Contact c, const Message& msg);
    void read_from(Contact c, Message& buffer);
  
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

  using RSCLocalCom = RSCLocalComImpl<IPC>;

}  // rsclocalcom

#endif /* RSCLOCAL_COM_H */

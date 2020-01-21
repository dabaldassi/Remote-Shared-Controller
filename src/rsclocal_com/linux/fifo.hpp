#ifndef FIFO_H
#define FIFO_H

#include <sstream>
#include <errno.h>

namespace rsclocalcom {

  /**
   *\class Fifo
   *\brief Implement a communication through Linux named pipe.
   */
  
  class Fifo
  {
  public:
    enum class Contact { CORE, CLIENT }; // Possible contact
  private:
    int     _fd_cmd, _fd_answer;
    Contact _self;

    static constexpr char CMD_FILE[] = "/var/rsc/localcom/fifo_cmd";
    static constexpr char ACK_FILE[] = "/var/rsc/localcom/fifo_ack";
    static constexpr int DEFAULT_READ_SIZE = 1024;
    static constexpr int DEFAULT_MODE = 0755; // Unix permission

  public:
  
    explicit Fifo(Contact c);

    /**
     *\brief Open the right file for the communication
     *\return 1 if there was an error. 0 otherwise
     */
    
    int  open();

    /**
     *\brief Send a message to a contact.
     *\param c The contact
     *\param msg The message as std::string
     *\return Negative value if error. The number of bytes written otherwise.
     */
    
    int  send_to(Contact c, const std::string& msg);

    /**
     *\brief Read a message from a contact
     *\param c The contact
     *\param msg The buffer in which will be stored the message
     *\return Negative value if error. The number of bytes read otherwise.
     */
    
    int  read_from(Contact c, std::string& answer);

    /**
     *\brief Send a message to the other side of the fifo.
     *\param msg The message as std::string
     *\return Negative value if error. The number of bytes written otherwise.
     */
     
    int send(const std::string& msg);

    /**
     *\brief Read a message from the other side of the fifo
     *\param msg The buffer in which will be stored the message
     *\return Negative value if error. The number of bytes read otherwise.
     */
    
    int read(std::string& answer);

    /**
     *\brief Close the connection
     */
    
    void close();
  
    ~Fifo();
  };

}  // rsclocalcom

#endif /* FIFO_H */

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
    int _fd_cmd, _fd_answer;

    static constexpr char PATH_CMD[] = "/tmp/fifo_cmd"; // TODO : set the real path
    static constexpr char PATH_ANSWER[] = "/tmp/fifo_answer"; // TODO : real path
    static constexpr int DEFAULT_READ_SIZE = 1024;
    static constexpr int DEFAULT_MODE = 0755; // Unix permission

  public:

    enum class Contact { CORE, CLIENT }; // Possible contact
  
    Fifo();

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
    
    int  read_from(Contact c, std::string& msg);


    /**
     *\brief Close the connection
     */
    
    void close();
  
    ~Fifo();
  };

}  // rsclocalcom

#endif /* FIFO_H */

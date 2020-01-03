#ifndef FIFO_H
#define FIFO_H

#include <sstream>
#include <errno.h>

namespace rsclocalcom {
  
  class Fifo
  {
    int _fd_cmd, _fd_answer;

    static constexpr char PATH_CMD[] = "/tmp/fifo_cmd";
    static constexpr char PATH_ANSWER[] = "/tmp/fifo_answer";
    static constexpr int DEFAULT_READ_SIZE = 1024;
    static constexpr int DEFAULT_MODE = 0755;

  public:

    enum class Contact { CORE, CLIENT };
  
    Fifo();

    int  open();
    int  send_to(Contact c, const std::string& msg);
    int  read_from(Contact c, std::string& msg);
    void close();
  
    ~Fifo();
  };

}  // rsclocalcom

#endif /* FIFO_H */

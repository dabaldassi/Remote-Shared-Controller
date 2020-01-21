#include <iostream>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <linux/fifo.hpp>

extern int errno;

using rsclocalcom::Fifo;

constexpr char Fifo::CMD_FILE[];
constexpr char Fifo::ACK_FILE[];

Fifo::Fifo(Contact c)
  : _fd_cmd{-1}, _fd_answer{-1}, _self{c}
{
  if(c == Contact::CORE) {
    int err = mkfifo(ACK_FILE, DEFAULT_MODE);
    if(err && errno != EEXIST) {
      std::string error = strerror(errno);
      throw std::runtime_error("Can't make fifo: " + error);
    }

    err = mkfifo(CMD_FILE, DEFAULT_MODE);
    if(err && errno != EEXIST) {
      std::string error = strerror(errno);
      throw std::runtime_error("Can't make fifo: " + error);
    }
  }
}

int Fifo::open()
{
  _fd_answer = ::open(ACK_FILE, O_RDWR);
  if(_fd_answer < 0) {
    perror("Can't open fifo ack");
    return 1;
  }
  
  _fd_cmd = ::open(CMD_FILE, O_RDWR);
  if(_fd_cmd < 0) {
    perror("Can't open fifo command");
    std::cerr << "Can't open : " << CMD_FILE << "\n";
    return 1;
  }

  return 0;
}

int Fifo::send_to(Contact c, const std::string & msg)
{
  int fd = (c == Contact::CLIENT)? _fd_cmd : _fd_answer;

  ssize_t size = write(fd, msg.c_str(), msg.size());
  
  return size;
}

int Fifo::read_from(Contact c, std::string& answer)
{
  int fd = (c == Contact::CORE)? _fd_cmd : _fd_answer;
  char msg[DEFAULT_READ_SIZE];

  bzero(msg, DEFAULT_READ_SIZE);
  size_t size = ::read(fd, msg, DEFAULT_READ_SIZE);

  if(size > 0) answer = msg;
  
  return size;
}

int Fifo::send(const std::string & msg)
{
  int fd = (_self == Contact::CLIENT)? _fd_cmd : _fd_answer;

  ssize_t size = write(fd, msg.c_str(), msg.size());
  
  return size;
}

int Fifo::read(std::string& answer)
{
  int fd = (_self == Contact::CORE)? _fd_cmd : _fd_answer;
  char msg[DEFAULT_READ_SIZE];

  bzero(msg, DEFAULT_READ_SIZE);
  size_t size = ::read(fd, msg, DEFAULT_READ_SIZE);

  if(size > 0) answer = msg;
  
  return size;
}

void Fifo::close()
{
  if(_fd_answer != -1) {
    ::close(_fd_answer);
    _fd_answer = -1;
  }

  if(_fd_cmd) {
    ::close(_fd_cmd);
    _fd_cmd = -1;
  }
}

Fifo::~Fifo()
{
  if(_fd_answer != -1 || _fd_cmd != -1) close();
  if(_self == Contact::CORE) {
    remove(ACK_FILE);
    remove(CMD_FILE);
  }
}

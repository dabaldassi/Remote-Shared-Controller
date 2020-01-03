#include <iostream>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/fifo.hpp>

extern int errno;

using rsclocalcom::Fifo;

constexpr char Fifo::PATH_CMD[];
constexpr char Fifo::PATH_ANSWER[];

Fifo::Fifo()
  : _fd_cmd{-1}, _fd_answer{-1}
{
  int err = mkfifo(PATH_ANSWER, DEFAULT_MODE);

  if(err && errno != EEXIST) perror("Can't make fifo");

  err = mkfifo(PATH_CMD, DEFAULT_MODE);
  if(err && errno != EEXIST) perror("Can't make fifo");
}

int Fifo::open()
{
  _fd_answer = ::open(PATH_ANSWER, O_RDWR);
  if(_fd_answer < 0) {
    std::cerr << "Can't open : " << PATH_ANSWER;
    return 1;
  }
  
  _fd_cmd = ::open(PATH_CMD, O_RDWR);
  if(_fd_cmd < 0) {
    std::cerr << "Can't open : " << PATH_CMD;
    return 1;
  }

  return 0;
}

int Fifo::read_from(Contact c, std::string& answer)
{
  int fd = (c == Contact::CORE)? _fd_cmd : _fd_answer;
  char msg[DEFAULT_READ_SIZE];

  bzero(msg, DEFAULT_READ_SIZE);
  size_t size = read(fd, msg, DEFAULT_READ_SIZE);

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

int Fifo::send_to(Contact c, const std::string & msg)
{
  int fd = (c == Contact::CLIENT)? _fd_cmd : _fd_answer;

  ssize_t size = write(fd, msg.c_str(), msg.size());
  
  return size;
}

Fifo::~Fifo()
{
  if(_fd_answer != -1 || _fd_cmd != -1) close();
}

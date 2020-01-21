#include <fstream>

#include <config.hpp>
#include <util.hpp>

#ifdef __gnu_linux__

#include <unistd.h>
#include <csignal>
#include <cstring>
#include <cerrno>

void sig_hdl(int)
{
}

bool rscutil::is_core_running()
{
  std::ifstream ifs(RSC_PID_FILE);

  if(ifs.is_open()) {
    pid_t pid;

    ifs >> pid;
    ifs.close();
    
    int result = kill(pid, SIGUSR1);

    if(result == -1 && errno != ESRCH) {
      std::string error = strerror(errno);
      throw std::runtime_error("Checking failed: " + error);
    }
    
    return result == 0;
  }
  
  return false;
}

void rscutil::register_pid()
{
  if(is_core_running())
    throw std::runtime_error("Core is already running");

  std::ofstream ofs(RSC_PID_FILE);

  if(ofs.is_open()) {
    pid_t pid = getpid();
    ofs << pid;
    ofs.close();

    signal(SIGUSR1, sig_hdl);
  }
  else throw std::runtime_error(std::string("Can't open : ") + RSC_PID_FILE);  
}

#else

#error "System not handled"

#endif


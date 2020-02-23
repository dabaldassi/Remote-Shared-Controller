#include <fstream>

#include <config.hpp>
#include <util.hpp>

#ifdef __gnu_linux__

#include <unistd.h>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

void sig_hdl(int)
{
}

void rscutil::create_base_dir()
{
  umask(0000);
  int err = mkdir(RSC_BASE_PATH, 0755);
  if(err && errno != EEXIST) {
    throw std::runtime_error("Couldn't create rsc base directory : " +
			     std::string(strerror(errno)));
  }
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

  int fd = creat(RSC_PID_FILE, 0666);
  if(fd > 0) close(fd);
  else       perror("Can't open rsc pid file");
  
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

#include <windows.h>
#include <psapi.h>

void rscutil::create_base_dir()
{
  
}

bool rscutil::is_core_running()
{
  unsigned long aProcesses[1024], cbNeeded, cProcesses;
  const std::string pName = "remote-shared-controller.exe";

  if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    return false;

  cProcesses = cbNeeded / sizeof(unsigned long);
  for (unsigned int i = 0; i < cProcesses; i++)
    {
      if (aProcesses[i] == 0) continue;

      HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, aProcesses[i]);
      char buffer[50];
      GetModuleBaseName(hProcess, 0, buffer, 50);
      CloseHandle(hProcess);
      if (pName == buffer) return true;
    }
  return false;
}

void rscutil::register_pid()
{

}

#endif

void rscutil::serialize_string(std::ofstream& ofs, const std::string& str)
{
  size_t size = str.size();
  ofs.write((char *)&size, sizeof(size));
  ofs.write(str.c_str(), size);
}

void rscutil::deserialize_string(std::ifstream& ifs, std::string& str)
{
  size_t size;
  char * str_tmp;

  ifs.read((char*)&size, sizeof(size));
  str_tmp = new char[size+1];
  
  ifs.read(str_tmp, size);
  
  str_tmp[size] = 0;
  str = str_tmp;
  
  delete [] str_tmp;
}


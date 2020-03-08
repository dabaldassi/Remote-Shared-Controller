#include <iostream>
#include <cstring>
#include <rsc.hpp>
#include <scnp.h>
#include <util.hpp>

void print_help()
{
  std::cout << "Remote-Shared-Controller help" << std::endl << std::endl;
  std::cout << "-i if_index" << "\t" << "Specify the network interface" << std::endl;
  std::cout << "-k key" << "\t" << "Specify the key to encrypt data" << std::endl;
  std::cout << std::endl;
}

int main(int argc, char *argv[])
{  
  rscutil::create_base_dir();
  rscutil::register_pid();

  RSC rsc;
  int if_index = -1;
  std::string key;

  for(int i = 1; i < argc; ++i) {
    if(argv[i] == std::string("-k")) {
      if(++i >= argc)
	throw std::runtime_error("-k need one argument : the key");
      else {
	key = argv[i];
	if(key.size() < 2) {
	  throw std::runtime_error("The key must have 2 character minimum");
	}
      }
    }
    else if(argv[i] == std::string("-i")) {
      if(++i >= argc)
	throw std::runtime_error("-i need one argument : the index of the network interface");
      else if_index = atoi(argv[i]);
    }
    else if(argv[i] == std::string("-h")) {
      print_help();
      return 0;
    }
    else {
      std::cout << "Don't know argument : " << argv[i] << std::endl;
      print_help();
      return 1;
    }
  }

  rsc.init(if_index, key);
  do {
    rsc.run();
    if(rsc.is_paused()) rsc.wait_for_wakeup();
    
  }while(rsc.is_running());
  
  rsc.exit();
  
  return 0;
}

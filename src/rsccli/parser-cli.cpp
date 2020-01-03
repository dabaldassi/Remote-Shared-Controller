#include <iostream>
#include <parser-cli.hpp>

bool start_with(const std::string& str, const std::string delim)
{
  size_t pos = str.find(delim);

  return pos == 0;
}

int Parser::parse(std::stringstream &ss)
{
  std::string cmd;
  
  ss >> cmd;
  
  _cmd = Command::get_command(cmd);

  if(_cmd == nullptr) {
    std::cout << "No such command : " << cmd << "\n";
    Command::print_default_usage();
    return 1;
  }

  try {

    while(!ss.eof()) {
      std::string arg;
      ss >> arg;

      if(start_with(arg, Command::OPT_DELIM)) _cmd->add_opt(arg);
      else                                    _cmd->add_arg(arg);
    
    }
    
  } catch(std::runtime_error& e) {
    std::cout <<  e.what() << "\n";
    _cmd->print_usage();
    _cmd = nullptr;
    return 1;
  }
  
  return 0;
}

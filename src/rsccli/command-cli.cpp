#include <iostream>
#include <algorithm>

#include <config-cli.hpp>
#include <command-cli.hpp>

///////////////////////////////////////////////////////////////////////////////
//                              Static variable                              //
///////////////////////////////////////////////////////////////////////////////

constexpr char HelpCommand::_NAME[];
constexpr char ListCommand::_NAME[];
constexpr char AddCommand::_NAME[];
constexpr char VersionCommand::_NAME[];
constexpr char IfCommand::_NAME[];
constexpr char Command::_NAME[];

const std::string Command::OPT_DELIM = "-";

///////////////////////////////////////////////////////////////////////////////
//                                  Command                                  //
///////////////////////////////////////////////////////////////////////////////

Command::Ptr Command::get_command(const std::string &name)
{
  if(name == HelpCommand::get_name())         return std::make_unique<HelpCommand>();
  else if(name == ListCommand::get_name())    return std::make_unique<ListCommand>();
  else if(name == AddCommand::get_name())     return std::make_unique<AddCommand>();
  else if(name == VersionCommand::get_name()) return std::make_unique<VersionCommand>();
  else if(name == IfCommand::get_name())      return std::make_unique<IfCommand>();
  else                                        return nullptr;
}

void Command::print_default_usage()
{
  std::cout << "Usage : " << RSCCLI << " command [opts] [args]" << "\n";
}

///////////////////////////////////////////////////////////////////////////////
//                                HelpCommand                                //
///////////////////////////////////////////////////////////////////////////////

void HelpCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI << " " << _NAME << "\n";
}

void HelpCommand::add_arg(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take argument"));
}

void HelpCommand::add_opt(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take option"));
}

void HelpCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tPrint " << RSCCLI << " help.\n";
  std::cout << "\n";
}

int HelpCommand::execute()
{
  std::cout << RSCCLI << " help : " << "\n\n";

  Command::print_default_usage();

  std::cout << "\n";
  std::cout << "List of available commands : " << "\n\n";
  
  HelpCommand::print_help();
  ListCommand::print_help();
  AddCommand::print_help();
  VersionCommand::print_help();
  IfCommand::print_help();

  std::cout << "\n";
  
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//                                ListCommand                                //
///////////////////////////////////////////////////////////////////////////////

std::map<char, std::function<int(ListCommand*)>> ListCommand::_on_opt = {
  { 'c', [] (ListCommand * cmd) -> int { return cmd->listcurrent(); } },
  { 'a', [] (ListCommand * cmd) -> int { return cmd->listall(); } },
  { 'r', [] (ListCommand * cmd) -> int { return cmd->listrefresh(); } },
};

void ListCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI << " " << _NAME << " [ -c | -a | -r ]\n";
}

void ListCommand::add_arg(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take argument"));
}

void ListCommand::add_opt(const std::string& opt)
{
  const char * c = opt.c_str();

  for(size_t i = 1; i < opt.size(); i++) {
    if(_opts.size() >= _nb_opt)
      throw std::range_error(_NAME + std::string(" too many options"));
    
    auto it = _on_opt.find(c[i]);
    if(it != _on_opt.end()) {
      auto it_opt = std::find(_opts.begin(), _opts.end(), c[i]);
      if(it_opt == _opts.end()) _opts.push_back(c[i]);
    }
    else {
      throw std::runtime_error(std::string("No such option for list : -") + c[i]);
    }
  }
}

void ListCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tPrint the list of computers.\n";
  std::cout << "\t\t" << "-c" << "\tList the current list of computers." << "\n";
  std::cout << "\t\t" << "-a"
	    << "\tList all the informations about the current computers."
	    << "\n";
  std::cout << "\t\t" << "-r" << "\tRefresh and list every computer available." << "\n";
  std::cout << "\n";
}

int ListCommand::execute() 
{
  if(_opts.empty()) _opts.push_back('c');
  
  for(char c : _opts) {
    int err = _on_opt[c](this);

    if(err) return err;
  }
  
  return 0;
}

int ListCommand::listall()
{
  std::cout << "listall" << "\n";
  return 0;
}

int ListCommand::listrefresh()
{
  std::cout << "listrefresh" << "\n";
  return 0;
}

int ListCommand::listcurrent()
{
  std::cout << "listcurrent" << "\n";
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//                                 AddCommand                                //
///////////////////////////////////////////////////////////////////////////////

void AddCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI << " " << _NAME << " id [id2]\n";
}

void AddCommand::add_arg(const std::string& arg)
{
  if(_args.size() < _nb_arg) _args.push_back(arg);
  else throw std::range_error("Too many arguments");
}

void AddCommand::add_opt(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take option"));
}

void AddCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tAdd a computer to the current list by its id.\n";
  std::cout << "\t\t" << _NAME << " id" << "\t\tAdd a computer at the end of the list.\n";
  std::cout << "\t\t" << _NAME << " id1 id2" << "\tAdd a computer id1 before id2.\n";
  std::cout << "\n";
}

int AddCommand::execute()
{
  if(_args.empty()) {
    std::cerr << _NAME << " need at least one argument\n";
    print_usage();
    return 1;
  }

  if(_args.size() == 1) {
    std::cout << "add " << _args.front() <<  " at the end" << "\n";
  }
  else {
    std::cout << "add " << _args.front() << " before " << _args.back() << "\n";
  }
  
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//                               VersionCommand                              //
///////////////////////////////////////////////////////////////////////////////

void VersionCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI << " " << _NAME << "\n";
}

void VersionCommand::add_arg(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take argument"));
}

void VersionCommand::add_opt(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take option"));
}

void VersionCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tPrint the current version of " << RSCCLI << ".\n";
  std::cout << "\n";
}

int VersionCommand::execute()
{
  std::cout << "Current version : " << VERSION << "\n\n";
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IfCommand                                 //
///////////////////////////////////////////////////////////////////////////////

std::map<char, std::function<int(IfCommand*)>> IfCommand::_on_opt = {
  { char{SET}, [] (IfCommand * cmd) -> int { return cmd->set(); } },
  { char{LIST}, [] (IfCommand * cmd) -> int { return cmd->list(); } },
};

void IfCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI << " " << _NAME
	    << " {-" << SET << " id | -" << LIST << " }\n";
}

void IfCommand::add_arg(const std::string& arg)
{
  if(_opts.empty())
    throw std::runtime_error(_NAME + std::string(" need an option first"));

  if(_args.empty() && _opts.front() == 's') _args.push_back(arg);
  else throw std::range_error("Too many arguments");
}

void IfCommand::add_opt(const std::string& opt)
{
  const char * c = opt.c_str();

  for(size_t i = 1; i < opt.size(); i++) {
    if(_opts.size() >= _nb_opt)
      throw std::range_error(_NAME + std::string(" too many options"));
    
    auto it = _on_opt.find(c[i]);
    if(it != _on_opt.end()) {
      _opts.push_back(c[i]);
    }
    else {
      throw std::runtime_error(std::string("No such option for list : -") + c[i]);
    }
  }
}

void IfCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tOperations on the network interface.\n";
  std::cout << "\t\t" << "-" << SET << " id" << "\tSet the current network interface." << "\n";
  std::cout << "\t\t" << "-" << LIST << "\tList all the available network interface." << "\n";
  std::cout << "\n";
}

int IfCommand::execute() 
{
  if(_opts.empty()) {
    std::cerr << _NAME << " need at least one option.\n";
    print_usage();
    return 1;
  }

  if(_opts.front() == SET && _args.empty()) {
    std::cerr << _NAME << " with option -" << SET << " need one argument.\n";
    print_usage();
    return 1;
  }
  
  for(char c : _opts) {
    int err = _on_opt[c](this);

    if(err) return err;
  }
  
  return 0;
}

int IfCommand::set()
{
  std::cout << "set the interface : " << _args.front() << "\n";
  return 0;
}

int IfCommand::list()
{
  std::cout << "list network interface" << "\n";
  return 0;
}


#include <iostream>
#include <algorithm>

#include <config.hpp>
#include <command-cli.hpp>
#include <controller_op.hpp>

using namespace rscui;

///////////////////////////////////////////////////////////////////////////////
//                              Static variable                              //
///////////////////////////////////////////////////////////////////////////////

constexpr char HelpCommand::_NAME[];
constexpr char ListCommand::_NAME[];
constexpr char AddCommand::_NAME[];
constexpr char RemoveCommand::_NAME[];
constexpr char VersionCommand::_NAME[];
constexpr char IfCommand::_NAME[];
constexpr char StartCommand::_NAME[];
constexpr char StopCommand::_NAME[];
constexpr char PauseCommand::_NAME[];
constexpr char ShortcutCommand::_NAME[];
constexpr char OptionCommand::_NAME[];
constexpr char SwapCommand::_NAME[];
constexpr char KeyCommand::_NAME[];
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
  else if(name == RemoveCommand::get_name())  return std::make_unique<RemoveCommand>();
  else if(name == IfCommand::get_name())      return std::make_unique<IfCommand>();
  else if(name == PauseCommand::get_name())   return std::make_unique<PauseCommand>();
  else if(name == StartCommand::get_name())   return std::make_unique<StartCommand>();
  else if(name == StopCommand::get_name())    return std::make_unique<StopCommand>();
  else if(name == ShortcutCommand::get_name())return std::make_unique<ShortcutCommand>();
  else if(name == OptionCommand::get_name())  return std::make_unique<OptionCommand>();
  else if(name == SwapCommand::get_name())    return std::make_unique<SwapCommand>();
  else if(name == KeyCommand::get_name())     return std::make_unique<KeyCommand>();
  else                                        return nullptr;
}

void Command::print_default_usage()
{
  std::cout << "Usage : " << RSCCLI_NAME << " command [opts] [args]" << "\n";
}

///////////////////////////////////////////////////////////////////////////////
//                                HelpCommand                                //
///////////////////////////////////////////////////////////////////////////////

void HelpCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << "\n";
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
  std::cout << "\t" << _NAME << "\tPrint " << RSCCLI_NAME << " help.\n";
  std::cout << "\n";
}

int HelpCommand::execute(ctrl_op_t & ops)
{
  return ops.help();
}

///////////////////////////////////////////////////////////////////////////////
//                                ListCommand                                //
///////////////////////////////////////////////////////////////////////////////

std::map<char, std::function<int(ListCommand*,ctrl_op_t&)>> ListCommand::_on_opt =
  {
   { char{CURRENT}, [] (ListCommand * cmd, ctrl_op_t& ops) { return cmd->listcurrent(ops); } },
   { char{REFRESH}, [] (ListCommand * cmd, ctrl_op_t& ops) { return cmd->listrefresh(ops); } },
};

void ListCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << " [ -c | -a | -r ]\n";
}

void ListCommand::add_arg(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take argument"));
}

void ListCommand::add_opt(const std::string& opt)
{
  const char * c = opt.c_str();

  if(opt.empty()) throw std::runtime_error("Option is empty !");
  if(opt.substr(0, 1) != OPT_DELIM)
    throw std::runtime_error("Option must start with !" + OPT_DELIM);
  
  for(size_t i = 1; i < opt.size(); i++) {
    if(_opts.size() >= _nb_opt ||
       (_opts.size() == 1 && _all) ||
       (_opts.size() == 1 && !_all && c[i] != ALL) ||
       (_all && c[i] == ALL))
      throw std::range_error(_NAME + std::string(" too many options"));
    
    auto it = _on_opt.find(c[i]);
    if(it != _on_opt.end()) {
      auto it_opt = std::find(_opts.begin(), _opts.end(), c[i]);
      if(it_opt == _opts.end()) _opts.push_back(c[i]);
    }
    else if(c[i] == ALL) _all = true;
    else {
      throw std::runtime_error(std::string("No such option for list : -") + c[i]);
    }
  }
}

void ListCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tPrint the list of computers.\n";
  std::cout << "\t\t" << "-" << CURRENT << "\tList the current list of computers." << "\n";
  std::cout << "\t\t" << "-" << ALL
	    << "\tList all the informations about the current computers."
	    << "\n";
  std::cout << "\t\t" << "-" << REFRESH << "\tRefresh and list every computer available." << "\n";
  std::cout << "\n";
}

int ListCommand::execute(ctrl_op_t & ops) 
{
  if(_opts.empty()) {
    int err = _on_opt[char{CURRENT}](this,ops);
    if(err) return err;
  }
  
  for(char c : _opts) {
    int err = _on_opt[c](this, ops);
    if(err) return err;
  }
  
  return 0;
}

int ListCommand::listrefresh(ctrl_op_t& ops)
{
  return ops.listrefresh(_all);
}

int ListCommand::listcurrent(ctrl_op_t& ops)
{
  return ops.listcurrent(_all);
}

///////////////////////////////////////////////////////////////////////////////
//                                 AddCommand                                //
///////////////////////////////////////////////////////////////////////////////

void AddCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << " id [id2]\n";
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

int AddCommand::execute(ctrl_op_t & ops)
{
  if(_args.empty()) {
    std::cerr << _NAME << " need at least one argument\n";
    print_usage();
    return 1;
  }

  if(_args.size() == 1) return ops.add(_args.front());
  else                  return ops.add(_args.front(), _args.back());
}

///////////////////////////////////////////////////////////////////////////////
//                               RemoveCommand                               //
///////////////////////////////////////////////////////////////////////////////

void RemoveCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << " id\n";
}

void RemoveCommand::add_arg(const std::string& arg)
{
  if(_args.size() < _nb_arg) _args.push_back(arg);
  else throw std::range_error("Too many arguments");
}

void RemoveCommand::add_opt(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take option"));
}

void RemoveCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tRemove a computer to the current list by its id.\n";
  std::cout << "\t\t" << _NAME << " id" << "\t\tRemove a computer at the end of the list.\n";
  std::cout << "\n";
}

int RemoveCommand::execute(ctrl_op_t & ops)
{
  if(_args.empty()) {
    std::cerr << _NAME << " need one argument\n";
    print_usage();
    return 1;
  }

  return ops.remove(_args.front());
}

///////////////////////////////////////////////////////////////////////////////
//                               VersionCommand                              //
///////////////////////////////////////////////////////////////////////////////

void VersionCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << "\n";
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
  std::cout << "\t" << _NAME << "\tPrint the current version of " << RSCCLI_NAME << ".\n";
  std::cout << "\n";
}

int VersionCommand::execute(ctrl_op_t & ops)
{
  return ops.version();
}

///////////////////////////////////////////////////////////////////////////////
//                                 IfCommand                                 //
///////////////////////////////////////////////////////////////////////////////

std::map<char,
	 std::function<int(IfCommand*,ctrl_op_t&)>> IfCommand::_on_opt = {
  { char{SET}, [] (IfCommand * cmd, ctrl_op_t& ops) -> int { return cmd->set(ops); } },
  { char{LIST}, [] (IfCommand *, ctrl_op_t& ops) -> int { return ops.listif(); } },
  { char{GET}, [] (IfCommand *, ctrl_op_t& ops) -> int { return ops.getif(); } },
};

void IfCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME
	    << " {-" << SET << " id | -" << LIST << " | -" << GET << " }\n";
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

  if(opt.empty()) throw std::runtime_error("Option is empty !");
  if(opt.substr(0, 1) != OPT_DELIM)
    throw std::runtime_error("Option must start with !" + OPT_DELIM);
  
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
  std::cout << "\t\t" << "-" << GET << "\tGet the current network interface." << "\n";
  std::cout << "\t\t" << "-" << LIST << "\tList all the available network interface." << "\n";
  std::cout << "\n";
}

int IfCommand::execute(ctrl_op_t & ops) 
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
    int err = _on_opt[c](this,ops);

    if(err) return err;
  }
  
  return 0;
}

int IfCommand::set(ctrl_op_t & ops)
{
  return ops.setif(_args.front());
}

///////////////////////////////////////////////////////////////////////////////
//                                   Start                                   //
///////////////////////////////////////////////////////////////////////////////

void StartCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << "\n";
}

void StartCommand::add_arg(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take argument"));
}

void StartCommand::add_opt(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take option"));
}

void StartCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tStart the core if paused " << RSCCLI_NAME << ".\n";
  std::cout << "\n";
}

int StartCommand::execute(ctrl_op_t & ops)
{
  return ops.start();
}

///////////////////////////////////////////////////////////////////////////////
//                                   Stop                                   //
///////////////////////////////////////////////////////////////////////////////

void StopCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << "\n";
}

void StopCommand::add_arg(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take argument"));
}

void StopCommand::add_opt(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take option"));
}

void StopCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tStop the core " << RSCCLI_NAME << ".\n";
  std::cout << "\n";
}

int StopCommand::execute(ctrl_op_t & ops)
{
  return ops.stop();
}

///////////////////////////////////////////////////////////////////////////////
//                                   Pause                                   //
///////////////////////////////////////////////////////////////////////////////

void PauseCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << "\n";
}

void PauseCommand::add_arg(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take argument"));
}

void PauseCommand::add_opt(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take option"));
}

void PauseCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tPause the core if it is running " << RSCCLI_NAME << ".\n";
  std::cout << "\n";
}

int PauseCommand::execute(ctrl_op_t & ops)
{
  return ops.pause();
}

///////////////////////////////////////////////////////////////////////////////
//                              ShortcutCommand                              //
///////////////////////////////////////////////////////////////////////////////

std::map<char,
	 std::function<int(ShortcutCommand*,ctrl_op_t&)>> ShortcutCommand::_on_opt = {
  { char{SET}, [] (ShortcutCommand * cmd, ctrl_op_t& ops) { return cmd->set(ops); } },
  { char{LIST}, [] (ShortcutCommand * cmd, ctrl_op_t& ops) { return cmd->list(ops); } },
  { char{RESET}, [] (ShortcutCommand *, ctrl_op_t& ops) { return ops.reset_shortcut(); } },
};

void ShortcutCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME
	    << " {-" << SET << " name | -" << LIST << " [name] | -" << RESET <<  " }\n";
}

void ShortcutCommand::add_arg(const std::string& arg)
{
  if(_opts.empty())
    throw std::runtime_error(_NAME + std::string(" need an option first"));

  if(_args.empty() && _opts.front() != RESET) _args.push_back(arg);
  else throw std::range_error("Too many arguments");
}

void ShortcutCommand::add_opt(const std::string& opt)
{
  const char * c = opt.c_str();

  if(opt.empty()) throw std::runtime_error("Option is empty !");
  if(opt.substr(0, 1) != OPT_DELIM)
    throw std::runtime_error("Option must start with !" + OPT_DELIM);

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

void ShortcutCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tCustomize rsc shortcuts.\n";
  std::cout << "\t\t" << "-" << SET << " name" << "\tSet a shortcut." << "\n";
  std::cout << "\t\t" << "-" << LIST << "\tList all the shortcuts." << "\n";
  std::cout << "\t\t" << "-" << LIST << " name" << "\tList the specified shortcut." << "\n";
  std::cout << "\t\t" << "-" << RESET << "\tReset to the default shortcuts." << "\n";
  std::cout << "\n";
}

int ShortcutCommand::execute(ctrl_op_t & ops) 
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

  int err = _on_opt[_opts.front()](this,ops);

  return err;
}

int ShortcutCommand::set(ctrl_op_t & ops)
{
  return ops.set_shortcut(_args.front());
}

int ShortcutCommand::list(ctrl_op_t & ops)
{
  if(_args.empty()) return ops.list_shortcut();
  else              return ops.list_shortcut(_args.front());
}

///////////////////////////////////////////////////////////////////////////////
//                               OptionCommand                               //
///////////////////////////////////////////////////////////////////////////////

std::map<std::string, unsigned> OptionCommand::_option_id = {
  { "circular", ControllerOperation::CIRCULAR },
};

constexpr char OptionCommand::ENABLED[];
constexpr char OptionCommand::DISABLED[];

void OptionCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << " opt_name {enabled | disabled}\n";
}

void OptionCommand::add_arg(const std::string& arg)
{
  if(_args.size() < _nb_arg) _args.push_back(arg);
  else throw std::range_error("Too many arguments");

  if(_args.size() == 1) {
    auto it = _option_id.find(arg);
    if(it != _option_id.end()) _option_to_set = it->second;
    else throw std::runtime_error("Not a valid option");
  }
  else {
    if(arg == ENABLED)       _state_to_set = true;
    else if(arg == DISABLED) _state_to_set = false;
    else throw std::runtime_error("Not a valid state");
  }
}

void OptionCommand::add_opt(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take option"));
}

void OptionCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tSet the state of an option.\n";
  std::cout << "\t\t" << _NAME << " opt_name enabled"
	    << "\tEnable the option with the name \"opt_name\".\n";
  std::cout << "\t\t" << _NAME << " opt_name disabled"
	    << "\tDisable the option with the name \"opt_name\".\n";
  std::cout << "\n";
}

int OptionCommand::execute(ctrl_op_t & ops)
{
  if(_args.size() < 2) {
    std::cerr << _NAME << " need exactly two arguments\n";
    print_usage();
    return 1;
  }

  return ops.set_option((ControllerOperation::Option)_option_to_set, _state_to_set);
}

///////////////////////////////////////////////////////////////////////////////
//                                SwapCommand                                //
///////////////////////////////////////////////////////////////////////////////

void SwapCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << " id1 id2\n";
}

void SwapCommand::add_arg(const std::string& arg)
{
  if(_args.size() < _nb_arg) _args.push_back(arg);
  else throw std::range_error("Too many arguments");
}

void SwapCommand::add_opt(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take option"));
}

void SwapCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tSwap two PC in the list.\n";
  std::cout << "\t\t" << _NAME << " id1 id2" << "\t. Swap PC with id1 with PC with id2\n";
  std::cout << "\n";
}

int SwapCommand::execute(ctrl_op_t & ops)
{
  if(_args.size() < 2) {
    std::cerr << _NAME << " need exactly two arguments\n";
    print_usage();
    return 1;
  }

  return ops.swap(std::stoi(_args.front()), std::stoi(_args.back()));
}

///////////////////////////////////////////////////////////////////////////////
//                                 KeyCommand                                //
///////////////////////////////////////////////////////////////////////////////

void KeyCommand::print_usage() const
{
  std::cout << "Usage : " << RSCCLI_NAME << " " << _NAME << " your_key\n";
}

void KeyCommand::add_arg(const std::string& arg)
{
  if(_args.size() < _nb_arg) _args.push_back(arg);
  else throw std::range_error("Too many arguments");
}

void KeyCommand::add_opt(const std::string&)
{
  throw std::runtime_error(_NAME + std::string(" doesn't take option"));
}

void KeyCommand::print_help()
{
  std::cout << "\t" << _NAME << "\tSwap two PC in the list.\n";
  std::cout << "\t\t" << _NAME << " id1 id2" << "\t. Swap PC with id1 with PC with id2\n";
  std::cout << "\n";
}

int KeyCommand::execute(ctrl_op_t & ops)
{
  if(_args.empty()) return ops.set_key();
  
  return ops.set_key(_args.front());
}

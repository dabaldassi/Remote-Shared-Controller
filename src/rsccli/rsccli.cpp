#include <iostream>
#include <sstream>

#include <rsccli.hpp>
#include <parser-cli.hpp>
#include <config.hpp>

#include <interface.h>
#include <pc_list.hpp>
#include <util.hpp>

using rscui::RSCCli;

int RSCCli::run(int argc, char **argv)
{
  Parser            parser;
  std::stringstream ss;

  for(int i = 1; i < argc; ++i) ss << " " << argv[i];

  int err = parser.parse(ss);

  if(err) return err;

  auto& cmd = parser.get_cmd();
  
  return cmd->execute(_ops);
}

void RSCCli::display_pc(const std::string& msg, rscutil::PCList& list, bool all)
{
  std::cout << msg << "\n";
  
  for(size_t i = 0; i < list.size(); i++) {
    const rscutil::PC& pc = list.get_current();
    
    if(all) std::cout << pc << "\n";
    else    std::cout << pc.name << "\t" << pc.id << "\n";
    
    list.next_pc();
  }
}

void RSCCli::display_all_pc(rscutil::PCList &list, bool all)
{
  display_pc("List of current PC :", list, all);
}

void RSCCli::display_current_pc(rscutil::PCList& list, bool all)
{
  display_pc("List of all available PC :", list, all);
}

void RSCCli::display_if(const IF * interface)
{
  std::cout << "Network interface : " << "\n\n";

  for(const IF* i = interface; !(i->if_index == 0 && i->if_name == NULL); ++i) {
    std::cout << "\t" << i->if_index << "\t" << i->if_name << "\n";
  }

  std::cout << "\n";
}

void RSCCli::display_error(const std::string& error)
{
  std::cerr << "[ERROR] " << error << "\n";
}

void RSCCli::display_version(const std::string& version)
{
  std::cout << "Current rsc version : " << version << "\n";
  std::cout << "Current rsccli version: " << RSCCLI_VERSION << "\n\n";
}

void RSCCli::display_help()
{
  std::cout << RSCCLI_NAME << " help : " << "\n\n";

  Command::print_default_usage();

  std::cout << "\n";
  std::cout << "List of available commands : " << "\n\n";
  
  HelpCommand::print_help();
  ListCommand::print_help();
  AddCommand::print_help();
  RemoveCommand::print_help();
  VersionCommand::print_help();
  IfCommand::print_help();
  StartCommand::print_help();
  StopCommand::print_help();
  PauseCommand::print_help();
  ShortcutCommand::print_help();

  std::cout << "\n";
}

void RSCCli::display_if(int if_index, const std::string &if_name)
{
  std::cout << "The current network interface is : \n";
  std::cout << if_index << "\t" << if_name << "\n";
}

void RSCCli::display_shortcut(rscutil::ComboShortcut::ComboShortcutList& list)
{
  std::cout << "Shortcut list : " << "\n\n";

  for(const auto& a : list) {
    std::cout << a.get_name() << "\t"
	      << a.get_description() << "\t"
	      << a.to_string() << "\n";
  }
}

void RSCCli::display_shortcut(rscutil::ComboShortcut& shortcut)
{
  std::cout << shortcut.get_name() << "\t"
	    << shortcut.get_description() << "\t"
	    << shortcut.to_string() << "\n";
}

bool RSCCli::shortcut_validation(const std::string & shortcut)
{
  std::cout << "Is this shortcut ok ?" << "\n";
  std::cout << shortcut << "\n";
  std::cout << "[y/N]" << "\n";

  char c = std::getchar();
  
  std::cout << (c == 'y') << " " << c << "\n";
  
  return c == 'y';
}

void RSCCli::prepare_shortcut()
{
  std::cout << "Press S to start the shortcut : " << "\n";
}

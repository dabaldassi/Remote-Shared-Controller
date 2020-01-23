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

void RSCCli::display_pc(rscutil::PCList& list, bool all)
{
  std::cout << "List of current PC :" << "\n";
  
  for(size_t i = 0; i < list.size(); i++) {
    const rscutil::PC& pc = list.get_current();
    
    if(all) std::cout << pc << "\n";
    else    std::cout << pc.name << "\t" << pc.id << "\n";
    
    list.next_pc();
  }
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
  std::cerr << error << "\n";
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

  std::cout << "\n";
}

void RSCCli::display_if(int if_index, const std::string &if_name)
{
  std::cout << "The current network interface is : \n";
  std::cout << if_index << "\t" << if_name << "\n";
}

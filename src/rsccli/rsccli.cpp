#include <iostream>
#include <sstream>

#include <rsccli.hpp>
#include <parser-cli.hpp>
#include <config-cli.hpp>

#include <interface.h>

int RSCCli::run(int argc, char **argv)
{
  Parser            parser;
  std::stringstream ss;

  for(int i = 1; i < argc; ++i) ss << " " << argv[i];

  int err = parser.parse(ss);

  if(err) return err;

  auto& cmd = parser.get_cmd();
  
  return cmd->execute(this);
}

int RSCCli::listall()
{
  std::cout << "listall" << "\n";
  return 0;
}

int RSCCli::listcurrent()
{
  std::cout << "listcurrent" << "\n";
  return 0;
}

int RSCCli::listrefresh()
{
  std::cout << "listrefresh" << "\n";
  return 0;
}

int RSCCli::add(const std::string & id)
{
  std::cout << "add " << id << " at the end" << "\n";
  return 0;
}

int RSCCli::add(const std::string & id1, const std::string & id2)
{
  std::cout << "add " << id1 << " before " << id2 << "\n";
  return 0;
}

int RSCCli::version()
{
  std::cout << "Current version : " << VERSION << "\n\n";
  return 0;
}

int RSCCli::help()
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

int RSCCli::setif(const std::string & id)
{
  std::cout << "set the interface : " << id << "\n";
  return 0;
}

int RSCCli::listif()
{
  IF * interface = get_interfaces();

  std::cout << "Network interface : " << "\n\n";

  if(interface) {
    for(IF* i = interface; !(i->if_index == 0 && i->if_name == NULL); ++i) {
      std::cout << "\t" << i->if_index << "\t" << i->if_name << "\n";
    }

    std::cout << "\n";
    free_interfaces(interface);
  }
    
  return 0;
}

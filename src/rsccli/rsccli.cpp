#include <iostream>
#include <sstream>

#include <rsccli.hpp>
#include <parser-cli.hpp>
#include <config-cli.hpp>

#include <interface.h>
#include <pc_list.hpp>

std::map<rsclocalcom::Message::Ack, std::function<void(void)>> RSCCli::_err_msg =
  { 
   { rsclocalcom::Message::ERROR, []() { std::cerr << "An error occured\n"; } },
   { rsclocalcom::Message::FUTURE, []() { std::cerr << "Not Implemented yet\n"; } },
  };

int RSCCli::_send_cmd(const rsclocalcom::Message& msg)
{
  using namespace rsclocalcom;
  Message m;
  
  _com.send_to(RSCLocalCom::Contact::CORE, msg);
  m.reset();
  _com.read_from(RSCLocalCom::Contact::CORE, m);

  if(m.get_cmd() == Message::ACK) {
    auto err = (Message::Ack)std::stoi(m.get_arg(0));

    if(err != Message::Ack::OK) {
      _err_msg[err]();
      return 1;
    }
    
    return 0;
  }
  else {
    std::cerr << "An error occured\n";
  }
	      
  return 1;
}

int RSCCli::_getlist(PCList& list, const std::string& file_name)
{
  rsclocalcom::Message msg(rsclocalcom::Message::GETLIST);

  int err = _send_cmd(msg);

  if(!err) list.load(file_name);

  return err;
}

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
  PCList list;
  _getlist(list, CURRENT_PC_LIST);

  std::cout << "List of current PC :" << "\n";
  
  for(size_t i = 0; i < list.size(); i++) {
    std::cout << list.get_current() << "\n";
    list.next_pc();
  }
  
  return 0;
}

int RSCCli::listcurrent()
{
  PCList list;
  _getlist(list, CURRENT_PC_LIST);

  std::cout << "List of current PC :" << "\n";
  
  for(size_t i = 0; i < list.size(); i++) {
    const PC& pc = list.get_current();
    std::cout << pc.name << "\t" << pc.id << "\n";
    list.next_pc();
  }
  
  return 0;
}

int RSCCli::listrefresh()
{
  PCList list;
  _getlist(list, ALL_PC_LIST);

  std::cout << "Available PC :" << "\n";
  
  for(size_t i = 0; i < list.size(); i++) {
    const PC& pc = list.get_current();
    std::cout << pc.name << "\t" << pc.id << "\n";
    list.next_pc();
  }
  
  return 0;
}

int RSCCli::add(const std::string & id)
{
  PCList all_list, current_list;

  _getlist(all_list, ALL_PC_LIST);
  current_list.load(CURRENT_PC_LIST);

  try {
    const PC& pc = all_list.get(std::stoi(id));
    current_list.add(pc);
  }catch(std::runtime_error& e) {
    std::cout << e.what() << "\n";
    return 1;
  }

  current_list.save(CURRENT_PC_LIST);
  
  rsclocalcom::Message msg(rsclocalcom::Message::SETLIST);
  return _send_cmd(msg);
}

int RSCCli::add(const std::string & id1, const std::string & id2)
{
   PCList all_list, current_list;

  _getlist(all_list, ALL_PC_LIST);
  current_list.load(CURRENT_PC_LIST);

  try {
    const PC& pc = all_list.get(std::stoi(id1));
    current_list.add(pc,std::stoi(id2));
  }catch(std::runtime_error& e) {
    std::cout << e.what() << "\n";
    return 1;
  }

  current_list.save(CURRENT_PC_LIST);
  rsclocalcom::Message msg(rsclocalcom::Message::SETLIST);
  return _send_cmd(msg);  
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
  RemoveCommand::print_help();
  VersionCommand::print_help();
  IfCommand::print_help();

  std::cout << "\n";
  
  return 0;
}

int RSCCli::setif(const std::string & id)
{
  rsclocalcom::Message msg(rsclocalcom::Message::IF);
  msg.add_arg(id);

  return _send_cmd(msg);
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

int RSCCli::remove(const std::string &id)
{
   PCList current_list;

  _getlist(current_list, CURRENT_PC_LIST);

  std::cout << current_list.size() << "\n";
  current_list.remove(std::stoi(id));
  std::cout << current_list.size() << "\n";
  
  current_list.save(CURRENT_PC_LIST);
  rsclocalcom::Message msg(rsclocalcom::Message::SETLIST);
  return _send_cmd(msg);
}

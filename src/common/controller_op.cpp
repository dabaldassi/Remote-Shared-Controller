#include <controller_op.hpp>
#include <util.hpp>
#include <pc_list.hpp>
#include <config.hpp>

using rscui::ControllerOperation;
using rscutil::PCList;
using rscutil::PC;

std::map<rsclocalcom::Message::AckCode, std::string> ControllerOperation::_err_msg =
  { 
   { rsclocalcom::Message::DEFAULT, "An error occured\n"},
   { rsclocalcom::Message::STARTED, "Already running\n" },
   { rsclocalcom::Message::PAUSED, "Core is paused\n"},
   { rsclocalcom::Message::FUTURE, "Not Implemented yet\n"},
  };

int ControllerOperation::_send_cmd(const rsclocalcom::Message& msg)
{
  using namespace rsclocalcom;
  Message     m;

  if(!rscutil::is_core_running()) throw std::runtime_error("Core is not running");

  RSCLocalCom com(RSCLocalCom::Contact::CLIENT);

  com.send(msg);
  m.reset();
  com.read(m);

  if(m.get_cmd() == Message::ACK) {
    auto ack = (Message::AckType)std::stoi(m.get_arg(0));

    if(ack != Message::AckType::OK) {
      auto err = (Message::AckCode)std::stoi(m.get_arg(1));
      _ui->display_error(_err_msg[err]);
      return -1;
    }

    int ret = std::stoi(m.get_arg(1));
    
    return ret;
  }
  else {
    _ui->display_error("An error occured\n");
  }
	      
  return -1;
}

int ControllerOperation::_getlist(PCList& list, const std::string& file_name)
{
  rsclocalcom::Message msg(rsclocalcom::Message::GETLIST);

  int err = _send_cmd(msg);

  if(!err) list.load(file_name);

  return err;
}

int ControllerOperation::listcurrent(bool all)
{
  PCList list;
  int    err = _getlist(list, CURRENT_PC_LIST);

  if(err) return err;
  
  _ui->display_pc(list, all);
  
  return 0;
}

int ControllerOperation::listrefresh(bool all)
{
  PCList list;
  int    err = _getlist(list, ALL_PC_LIST);

  if(err) return err;
  
  _ui->display_pc(list, all);
  
  return 0;
}

int ControllerOperation::add(const std::string & id)
{
  PCList all_list, current_list;
  int    err = _getlist(all_list, ALL_PC_LIST);

  if(err) return err;
  
  current_list.load(CURRENT_PC_LIST);

  try {
    const PC& pc = all_list.get(std::stoi(id));
    current_list.add(pc);
  }catch(std::runtime_error& e) {
    _ui->display_error(e.what());
    return 1;
  }

  current_list.save(CURRENT_PC_LIST);
  
  rsclocalcom::Message msg(rsclocalcom::Message::SETLIST);
  return _send_cmd(msg);
}

int ControllerOperation::add(const std::string & id1, const std::string & id2)
{
   PCList all_list, current_list;
   int    err = _getlist(all_list, ALL_PC_LIST);

   if(err) return err;
   
   current_list.load(CURRENT_PC_LIST);

   try {
     const PC& pc = all_list.get(std::stoi(id1));
     current_list.add(pc,std::stoi(id2));
   }catch(std::runtime_error& e) {
     _ui->display_error(e.what());
     return 1;
   }

   current_list.save(CURRENT_PC_LIST);
   rsclocalcom::Message msg(rsclocalcom::Message::SETLIST);
   return _send_cmd(msg);  
}

int ControllerOperation::version()
{
  _ui->display_version(RSC_VERSION);
  return 0;
}

int ControllerOperation::help()
{
  _ui->display_help();
  
  return 0;
}

int ControllerOperation::setif(const std::string & id)
{
  rsclocalcom::Message msg(rsclocalcom::Message::IF);
  msg.add_arg(id);

  return _send_cmd(msg);
}

int ControllerOperation::listif()
{
  IF * interface = get_interfaces();

  if(interface) {
    _ui->display_if(interface);
    free_interfaces(interface);
    return 0;
  }
    
  return 1;
}

int ControllerOperation::remove(const std::string &id)
{
   PCList current_list;
   int    err = _getlist(current_list, CURRENT_PC_LIST);

   if(err) return err;

   current_list.remove(std::stoi(id));
  
   current_list.save(CURRENT_PC_LIST);
   rsclocalcom::Message msg(rsclocalcom::Message::SETLIST);
   return _send_cmd(msg);
}

int ControllerOperation::start()
{
  rsclocalcom::Message msg(rsclocalcom::Message::START);

  return _send_cmd(msg);
}

int ControllerOperation::stop()
{
  rsclocalcom::Message msg(rsclocalcom::Message::STOP);

  return _send_cmd(msg);
}

int ControllerOperation::pause()
{
  rsclocalcom::Message msg(rsclocalcom::Message::PAUSE);

  return _send_cmd(msg);
}

int ControllerOperation::getif()
{
  rsclocalcom::Message msg(rsclocalcom::Message::GETIF);

  int ret = _send_cmd(msg);
  
  if(ret > 0) {
    unsigned int if_index = ret;
    IF *         ifs = get_interfaces();

    if(!ifs) {
      _ui->display_error("Can not get network interface list");
      return 1;
    }

    while(!(ifs->if_index == 0 && ifs->if_name == NULL) && (if_index != ifs->if_index)) {
      ++ifs;
    }

    if(ifs->if_index == 0 && ifs->if_name == NULL) {
      _ui->display_error("The current interface does not exist");
      return 1;
    }
    
    _ui->display_if(if_index, ifs->if_name);
    free_interfaces(ifs);
    return 0;
  }

  return 1;
}

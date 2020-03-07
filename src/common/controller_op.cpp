#include <util.hpp>
#include <pc_list.hpp>
#include <config.hpp>
#include <combo.hpp>

#include <controller_op.hpp>

using rscui::ControllerOperation;
using rscutil::PCList;
using rscutil::PC;
using rscutil::ComboShortcut;

std::map<rsclocalcom::Message::AckCode, std::string> ControllerOperation::_err_msg =
  { 
   { rsclocalcom::Message::DEFAULT, "An error occured"},
   { rsclocalcom::Message::STARTED, "Already running" },
   { rsclocalcom::Message::PAUSED, "Core is paused"},
   { rsclocalcom::Message::FUTURE, "Not Implemented yet"},
   { rsclocalcom::Message::IF_EXIST, "This is not a valid interface"},
  };

int ControllerOperation::_send_cmd(const rsclocalcom::Message& msg)
{
  using namespace rsclocalcom;
  Message     m;

  if(!rscutil::is_core_running()) {
    _ui->display_error("Core is not running");
    return 1;
  }

  RSCLocalCom com(RSCLocalCom::Contact::CLIENT);

  if (com.send(msg) < 0) {
      _ui->display_error("Can't send message to rsc core");
      return -1;
  }

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
    _ui->display_error("An error occured");
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

int ControllerOperation::_get_shortcut(ComboShortcut::ComboShortcutList& list)
{
  rsclocalcom::Message msg(rsclocalcom::Message::SAVE_SHORTCUT);

  int err = _send_cmd(msg);

  if(!err) ComboShortcut::load(list);

  return err;
}

int ControllerOperation::listcurrent(bool all)
{
  PCList list;
  int    err = _getlist(list, CURRENT_PC_LIST);

  if(err) return err;
  
  _ui->display_current_pc(list, all);
  
  return 0;
}

int ControllerOperation::listrefresh(bool all)
{
  PCList list;
  int    err = _getlist(list, ALL_PC_LIST);

  if(err) return err;
  
  _ui->display_all_pc(list, all);
  
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

    IF * if_cur = ifs;
    
    while(!(if_cur->if_index == 0 && if_cur->if_name == NULL) && (if_index != if_cur->if_index)){
      ++if_cur;
    }

    if(if_cur->if_index == 0 && if_cur->if_name == NULL) {
      _ui->display_error("The current interface does not exist");
      return 1;
    }
    
    _ui->display_if(if_index, if_cur->if_name);
    free_interfaces(ifs);
    return 0;
  }

  return 1;
}

int ControllerOperation::list_shortcut()
{
  ComboShortcut::ComboShortcutList list;

  int err = _get_shortcut(list);
  if(err) return err;

  _ui->display_shortcut(list);
  
  return 0;
}

int ControllerOperation::list_shortcut(const std::string& name)
{
  ComboShortcut::ComboShortcutList list;

  int err = _get_shortcut(list);
  if(err) return err;

  auto it = std::find_if(list.begin(),
			 list.end(),
			 [&name](const auto& a) { return name == a.get_name(); });

  if(it == list.end()) {
    _ui->display_error("This shortcut does not exist");
    return 1;
  }

  _ui->display_shortcut(*it);

  return 0;
}

int ControllerOperation::set_shortcut(const std::string& name)
{
  ComboShortcut::ComboShortcutList list;

  int err = _get_shortcut(list);
  if(err) return err;

  auto it = std::find_if(list.begin(),
			 list.end(),
			 [&name](const auto& a) { return name == a.get_name(); });

  if(it == list.end()) {
    _ui->display_error("This shortcut does not exist");
    return 1;
  }

  ComboShortcut new_combo(it->get_name(), it->get_description());

  _ui->prepare_shortcut();
  ComboShortcut::make_shortcut(new_combo);

  bool validation = _ui->shortcut_validation(new_combo.to_string());

  if(validation) {
    list.emplace(it, new_combo);
    list.erase(it);

    ComboShortcut::save(list);

    rsclocalcom::Message msg(rsclocalcom::Message::LOAD_SHORTCUT);
    msg.add_arg(int{rsclocalcom::Message::LOAD_DEFAULT});
    return _send_cmd(msg);
  }
  
  return 0;
}

int ControllerOperation::reset_shortcut()
{
  rsclocalcom::Message msg(rsclocalcom::Message::LOAD_SHORTCUT);
  msg.add_arg(int{rsclocalcom::Message::LOAD_RESET});
  return _send_cmd(msg);
}

int ControllerOperation::swap(int id1, int id2)
{
   PCList current_list;
   int    err = _getlist(current_list, CURRENT_PC_LIST);

   if(err) return err;

   current_list.swap(id1,id2);
  
   current_list.save(CURRENT_PC_LIST);
   rsclocalcom::Message msg(rsclocalcom::Message::SETLIST);
   return _send_cmd(msg);
}

int ControllerOperation::set_option(Option opt, bool state)
{
  rsclocalcom::Message msg;

  switch(opt) {
  case Option::CIRCULAR:
    msg.reset(rsclocalcom::Message::CIRCULAR);
    msg.add_arg(state);
    break;
  default:
    _ui->display_error("Not a valid option");
    return 1;
  }
  
  return _send_cmd(msg);
}

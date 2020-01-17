#include <algorithm>
#include <message.hpp>

using rsclocalcom::Message;

std::vector<std::tuple<std::string,size_t>> Message::_commands =
  { std::make_tuple("IF",1),
    std::make_tuple("GETLIST",0),
    std::make_tuple("SETLIST",0),
    std::make_tuple("ACK",1),
    std::make_tuple("START",0),
    std::make_tuple("STOP",0),
    std::make_tuple("PAUSE",0),
  };

Message::Message(Command c): _cmd(c)
{
  if(_cmd > NA) _cmd = NA;
}

void Message::get(std::stringstream &ss) const
{
  if(_cmd < NA) ss << std::get<0>(_commands[_cmd]);
  else throw std::runtime_error("Command is N/A");

  if(_args.size() != std::get<1>(_commands[_cmd])) {
    std::stringstream().swap(ss);
    throw std::runtime_error("Number of arguments don't match");
  }
    
  for(const auto& s : _args) ss << " " << s;
}

void Message::reset(Command c)
{
  _cmd = c;
  _args.clear();
}

void Message::set(std::stringstream& ss)
{
  std::string cmd;
  reset();
  ss >> cmd;

  for(size_t i = 0; i < _commands.size() && _cmd == NA; i++) {
    if(cmd == std::get<0>(_commands[i])) _cmd = (Command)(i);
  }

  if(_cmd != NA) {
    size_t len_arg = std::get<1>(_commands[_cmd]);

    while(_args.size() < len_arg && !ss.eof()) {
      std::string arg;
      ss >> arg;
      _args.push_back(arg);
    }
    
    if(_args.size() < len_arg) throw std::range_error("Missing arguments");
  }
  else throw std::runtime_error("This command does not exist");
}

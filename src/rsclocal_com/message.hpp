#ifndef MESSAGE_H
#define MESSAGE_H

#include <sstream>
#include <vector>
#include <tuple>

namespace rsclocalcom {

  class Message
  {
  public:
    enum Command{ IF, GETLIST, SETLIST, ACK, NA};
    
  private:
    Command                  _cmd;
    std::vector<std::string> _args;
    
    static std::vector<std::tuple<std::string,size_t>> _commands;

    template<typename T>
    std::string        _to_string(T&& t) { return std::to_string(std::forward<T>(t)); }
    const std::string& _to_string(const std::string& s) { return s; }
    const char *       _to_string(const char* s) { return s; }
    
  public:
    
    Message(Command c = NA);

    void get(std::stringstream& ss) const;
    void set(std::stringstream& ss);
    void reset(Command c = NA);

    template<typename T>
    void add_arg(T&& t) {
      if(_cmd == NA) throw std::runtime_error("Command is N/A");
      if(_args.size() >= std::get<1>(_commands[_cmd])) throw std::range_error("Too much arguments");

      _args.push_back(_to_string(std::forward<T>(t)));
    }

  };


}  // rsclocalcom

#endif /* MESSAGE_H */

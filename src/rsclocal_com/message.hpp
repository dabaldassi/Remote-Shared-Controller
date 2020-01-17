#ifndef MESSAGE_H
#define MESSAGE_H

#include <sstream>
#include <vector>
#include <tuple>

namespace rsclocalcom {

  class Message
  {
  public:
    enum Command{ IF, GETLIST, SETLIST, ACK, START, STOP, PAUSE, NA};
    enum Ack { OK, ERROR, STARTED, PAUSED, FUTURE };
    
  private:
    Command                  _cmd;
    std::vector<std::string> _args;
    
    static std::vector<std::tuple<std::string,size_t>> _commands;

    /**
     *\fn _to_string
     *\brief Convert a type to std::string if possible
     */
    
    template<typename T>
    std::string        _to_string(T&& t) { return std::to_string(std::forward<T>(t));}
    const std::string& _to_string(const std::string& s) { return s; }
    const char *       _to_string(const char* s) { return s; }
    
  public:
    
    Message(Command c = NA);

    /**
     *\brief Get the message's command
     *\return The command
     */
    
    Command get_cmd() const { return _cmd; }

    /**
     *\brief Get the message in a stringstream
     *\param ss The stringstream to get the message
     *\exception std::runtime_error if the command is NA or if the number of argument does not match
     */
    
    void get(std::stringstream& ss) const;

    /**
     *\brief Get the argument at the index i
     *\param i The argument's index
     *\return The argument as string
     */
    
    const std::string& get_arg(size_t i) const { return _args[i]; }

    /**
     *\brief Set a message from a stringstream
     *\remarks This will call the method reset
     *\param ss The stringstream containin the message to build
     *\exception std::range_error If argument are missing
     *\exception std::runtime_error If the command does not exist
     */
    
    void set(std::stringstream& ss);

    /**
     *\brief Reset the message. This will clear all the arguments and set a new command.
     * The default command is NA
     *\param c The new command.
     */
    
    void reset(Command c = NA);

    /**
     *\brief Add an argument to the message if possible.
     *\param t The argument. It will be converted to string only if possible.
     *\exception std::runtime_error Throw this exception if command if NA
     *\exception std::range_error Throw this exception if the number of argument is exeeded
     */
    
    template<typename T>
    void add_arg(T&& t) {
      if(_cmd == NA) throw std::runtime_error("Command is N/A");
      if(_args.size() >= std::get<1>(_commands[_cmd])) throw std::range_error("Too much arguments");

      _args.push_back(_to_string(std::forward<T>(t)));
    }

  };


}  // rsclocalcom

#endif /* MESSAGE_H */

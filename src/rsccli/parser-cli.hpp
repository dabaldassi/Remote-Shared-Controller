#ifndef PARSER_H
#define PARSER_H

#include <sstream>
#include <command-cli.hpp>

namespace rscui {

  class Parser
  {
    Command::Ptr _cmd;
  
  public:
    Parser()
      : _cmd(nullptr) {}

    Parser(const Parser&) = delete;

    /**
     *\brief Parse the command contained in the stringstream
     *\param ss The command as an std::stringstream
     *\return 1 if there was an error, 0 otherwise
     *\warning There must be no space at the end of the stringstream
     */
  
    int parse(std::stringstream& ss);

    /**
     *\brief Get the parsed command
     *\return The command to execute
     *\remark the method must be called before getting the command
     */
  
    const Command::Ptr& get_cmd() const { return _cmd; }
    Command::Ptr& get_cmd() { return _cmd; }
  
  };

}  // rscui

#endif /* PARSER_H */

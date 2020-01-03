#ifndef PARSER_H
#define PARSER_H

#include <sstream>
#include <command-cli.hpp>

class Parser
{
  Command::Ptr _cmd;
  
public:
  Parser()
    : _cmd(nullptr) {}

  Parser(const Parser&) = delete;

  int parse(std::stringstream& ss);

  const Command::Ptr& get_cmd() const { return _cmd; }
  Command::Ptr& get_cmd() { return _cmd; }
  
};


#endif /* PARSER_H */

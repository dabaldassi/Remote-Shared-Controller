#include <sstream>

#include <rsccli.hpp>
#include <parser-cli.hpp>

int RSCCli::run(int argc, char **argv)
{
  Parser            parser;
  std::stringstream ss;

  for(int i = 1; i < argc; ++i) ss << " " << argv[i];

  int err = parser.parse(ss);

  if(err) return err;

  auto& cmd = parser.get_cmd();
  
  return cmd->execute();
}

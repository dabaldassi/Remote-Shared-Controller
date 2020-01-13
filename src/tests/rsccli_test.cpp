#include "catch/catch.hpp"

#include <iostream>
#include <list>
#include <tuple>

#include <command-cli.hpp>
#include <parser-cli.hpp>
#include <rsccli.hpp>

enum { LISTALL, LISTCURRENT, LISTREFRESH, ADD, ADD2, REMOVE, SETIF, LISTIF, VERSION, HELP };

std::map<rsclocalcom::Message::Ack, std::function<void(void)>> RSCCli::_err_msg =
  { 
   { rsclocalcom::Message::ERROR, []() { std::cerr << "An error occured\n"; } },
   { rsclocalcom::Message::FUTURE, []() { std::cerr << "Not Implemented yet\n"; } },
  };

int RSCCli::_send_cmd(const rsclocalcom::Message& )
{
  return 0;
}

int RSCCli::_getlist(PCList&, const std::string& )
{
  return 0;
}

int RSCCli::run(int argc, char **argv)
{

  return 0;
}

int RSCCli::listall()
{
  return LISTALL;
}

int RSCCli::listcurrent()
{
  return LISTCURRENT;
}

int RSCCli::listrefresh()
{
  return LISTREFRESH;
}

int RSCCli::add(const std::string &)
{
  return ADD;
}

int RSCCli::add(const std::string &, const std::string &)
{
 
  return ADD2;
}

int RSCCli::version()
{
  return VERSION;
}

int RSCCli::help()
{

  return HELP;
}

int RSCCli::setif(const std::string &)
{
  return SETIF;
}

int RSCCli::listif()
{
  return LISTIF;
}

int RSCCli::remove(const std::string &)
{
  return REMOVE;
}

TEST_CASE("Command") {
  RSCCli cli;
  
  SECTION("list") {
    ListCommand cmd, cmd2, cmd3;

    REQUIRE(cmd.execute(&cli) == LISTCURRENT);

    REQUIRE_THROWS(cmd.add_arg("1"));
    REQUIRE_THROWS(cmd.add_opt("a"));
    REQUIRE_NOTHROW(cmd.add_opt("-a"));
    REQUIRE(cmd.execute(&cli) == LISTALL);
    REQUIRE_THROWS(cmd.add_opt("-c"));

    REQUIRE_THROWS(cmd2.add_opt("-e"));
    REQUIRE_NOTHROW(cmd2.add_opt("-r"));
    REQUIRE(cmd2.execute(&cli) == LISTREFRESH);

    REQUIRE_NOTHROW(cmd3.add_opt("-c"));
    REQUIRE(cmd3.execute(&cli) == LISTCURRENT);
  }

  SECTION("add") {
    AddCommand cmd;

    REQUIRE(cmd.execute(&cli) == 1); // ERROR

    REQUIRE_NOTHROW(cmd.add_arg("1"));
    REQUIRE_THROWS(cmd.add_opt("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    REQUIRE(cmd.execute(&cli) == ADD);

    REQUIRE_NOTHROW(cmd.add_arg("2"));
    REQUIRE(cmd.execute(&cli) == ADD2);

    REQUIRE_THROWS(cmd.add_arg("3"));
  }

  SECTION("version") {
    VersionCommand cmd;

    REQUIRE_THROWS(cmd.add_arg("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    
    REQUIRE(cmd.execute(&cli) == VERSION);
  }
  
  SECTION("help") {
    HelpCommand cmd;

    REQUIRE_THROWS(cmd.add_arg("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    
    REQUIRE(cmd.execute(&cli) == HELP);
  }

  SECTION("Remove") {
    RemoveCommand cmd;

    REQUIRE(cmd.execute(&cli) == 1); // ERROR

    REQUIRE_NOTHROW(cmd.add_arg("1"));
    REQUIRE_THROWS(cmd.add_opt("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    REQUIRE(cmd.execute(&cli) == REMOVE);

    REQUIRE_THROWS(cmd.add_arg("2"));
  }

  SECTION("IF") {
    IfCommand cmd, cmd2;

    REQUIRE(cmd.execute(&cli) == 1);

    REQUIRE_THROWS(cmd.add_arg("l"));
    REQUIRE_NOTHROW(cmd.add_opt("l"));
    REQUIRE_NOTHROW(cmd.add_opt("-l"));
    REQUIRE_THROWS(cmd.add_arg("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));

    REQUIRE(cmd.execute(&cli) == LISTIF);

    REQUIRE_NOTHROW(cmd2.add_opt("-s"));
    REQUIRE_THROWS(cmd2.add_opt("-s"));
    REQUIRE_NOTHROW(cmd2.add_arg("1"));
    REQUIRE_THROWS(cmd2.add_arg("2"));

    REQUIRE(cmd2.execute(&cli) == SETIF);
  }
}

TEST_CASE("parser") {
  RSCCli cli;
  Parser parser;
  
  std::list<std::tuple<std::string, int, int>> cmd_list =
    {
     std::make_tuple("list", 0, LISTCURRENT),
     std::make_tuple("list -c", 0, LISTCURRENT),
     std::make_tuple("list -a", 0, LISTALL),
     std::make_tuple("list -r", 0, LISTREFRESH),
     std::make_tuple("list -r -r", 1, 0),
     std::make_tuple("list abc", 1, 0),
     std::make_tuple("list -c a", 1, 0),
     std::make_tuple("list -z", 1, 0),
     std::make_tuple("add 1", 0, ADD),
     std::make_tuple("add 1 2", 0, ADD2),
     std::make_tuple("add 1 2 3", 1, 0),
     std::make_tuple("add", 0, 1),
     std::make_tuple("add -e", 1, 0),
     std::make_tuple("add 1 -a", 1, 0),
     std::make_tuple("add -a 1", 1, 0),
     std::make_tuple("add 1 2 -e", 1, 0),
     std::make_tuple("remove 1", 0, REMOVE),
     std::make_tuple("remove -a", 1, 0),
     std::make_tuple("remove", 0, 1),
     std::make_tuple("remove 1 1", 1, 0),
     std::make_tuple("remove 1 -a", 1, 0),
     std::make_tuple("remove -a 1", 1, 0),
     std::make_tuple("version", 0, VERSION),
     std::make_tuple("version -a", 1, 0),
     std::make_tuple("version a", 1, 0),
     std::make_tuple("help", 0, HELP),
     std::make_tuple("help -a", 1, 0),
     std::make_tuple("help a", 1, 0),
     std::make_tuple("if -l", 0, LISTIF),
     std::make_tuple("if -s 1", 0, SETIF),
     std::make_tuple("if -l 1", 1, 0),
     std::make_tuple("if", 0, 1),
     std::make_tuple("if -r", 1, 0),
     std::make_tuple("if -s 1 1", 1, 0),
     std::make_tuple("if 1 -s", 1, 0),
    };

  for(const auto& a: cmd_list) {
    std::stringstream ss(std::get<0>(a));

    std::cout << std::get<0>(a) << "\n";
    REQUIRE(parser.parse(ss) == std::get<1>(a));

    if(!std::get<1>(a)) {
      auto& cmd = parser.get_cmd();

      REQUIRE(cmd->execute(&cli) == std::get<2>(a));
    }
    else {
      REQUIRE(parser.get_cmd() == nullptr);
    }
  }
}

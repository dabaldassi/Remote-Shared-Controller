#include "catch/catch.hpp"

#include <iostream>
#include <list>
#include <tuple>

#include <command-cli.hpp>
#include <parser-cli.hpp>
#include <controller_op.hpp>

enum { LISTALL, LISTCURRENT, LISTREFRESH, ADD, ADD2, REMOVE, SETIF, LISTIF, VERSION, HELP,
       START, STOP, PAUSE};

using namespace rscui;

std::map<rsclocalcom::Message::Ack, std::string> ControllerOperation::_err_msg{};

int ControllerOperation::_send_cmd(const rsclocalcom::Message& )
{
  return 0;
}

int ControllerOperation::_getlist(rscutil::PCList&, const std::string& )
{
  return 0;
}

int ControllerOperation::listall()
{
  return LISTALL;
}

int ControllerOperation::listcurrent()
{
  return LISTCURRENT;
}

int ControllerOperation::listrefresh()
{
  return LISTREFRESH;
}

int ControllerOperation::add(const std::string &)
{
  return ADD;
}

int ControllerOperation::add(const std::string &, const std::string &)
{
 
  return ADD2;
}

int ControllerOperation::version()
{
  return VERSION;
}

int ControllerOperation::help()
{

  return HELP;
}

int ControllerOperation::setif(const std::string &)
{
  return SETIF;
}

int ControllerOperation::listif()
{
  return LISTIF;
}

int ControllerOperation::remove(const std::string &)
{
  return REMOVE;
}

int ControllerOperation::start()
{
  return START;
}

int ControllerOperation::stop()
{
  return STOP;
}

int ControllerOperation::pause()
{
  return PAUSE;
}


TEST_CASE("Command") {

  ControllerOperation ops(nullptr);
  
  SECTION("list") {
    ListCommand cmd, cmd2, cmd3;

    REQUIRE(cmd.execute(ops) == LISTCURRENT);

    REQUIRE_THROWS(cmd.add_arg("1"));
    REQUIRE_THROWS(cmd.add_opt("a"));
    REQUIRE_NOTHROW(cmd.add_opt("-a"));
    REQUIRE(cmd.execute(ops) == LISTALL);
    REQUIRE_THROWS(cmd.add_opt("-c"));

    REQUIRE_THROWS(cmd2.add_opt("-e"));
    REQUIRE_NOTHROW(cmd2.add_opt("-r"));
    REQUIRE(cmd2.execute(ops) == LISTREFRESH);

    REQUIRE_NOTHROW(cmd3.add_opt("-c"));
    REQUIRE(cmd3.execute(ops) == LISTCURRENT);
  }

  SECTION("add") {
    AddCommand cmd;

    REQUIRE(cmd.execute(ops) == 1); // ERROR

    REQUIRE_NOTHROW(cmd.add_arg("1"));
    REQUIRE_THROWS(cmd.add_opt("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    REQUIRE(cmd.execute(ops) == ADD);

    REQUIRE_NOTHROW(cmd.add_arg("2"));
    REQUIRE(cmd.execute(ops) == ADD2);

    REQUIRE_THROWS(cmd.add_arg("3"));
  }

  SECTION("version") {
    VersionCommand cmd;

    REQUIRE_THROWS(cmd.add_arg("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    
    REQUIRE(cmd.execute(ops) == VERSION);
  }

  SECTION("start") {
    StartCommand cmd;

    REQUIRE_THROWS(cmd.add_arg("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    
    REQUIRE(cmd.execute(ops) == START);
  }

  SECTION("stop") {
    StopCommand cmd;

    REQUIRE_THROWS(cmd.add_arg("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    
    REQUIRE(cmd.execute(ops) == STOP);
  }

    SECTION("pause") {
    PauseCommand cmd;

    REQUIRE_THROWS(cmd.add_arg("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    
    REQUIRE(cmd.execute(ops) == PAUSE);
  }
  
  SECTION("help") {
    HelpCommand cmd;

    REQUIRE_THROWS(cmd.add_arg("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    
    REQUIRE(cmd.execute(ops) == HELP);
  }

  SECTION("Remove") {
    RemoveCommand cmd;

    REQUIRE(cmd.execute(ops) == 1); // ERROR

    REQUIRE_NOTHROW(cmd.add_arg("1"));
    REQUIRE_THROWS(cmd.add_opt("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));
    REQUIRE(cmd.execute(ops) == REMOVE);

    REQUIRE_THROWS(cmd.add_arg("2"));
  }

  SECTION("IF") {
    IfCommand cmd, cmd2;

    REQUIRE(cmd.execute(ops) == 1);

    REQUIRE_THROWS(cmd.add_arg("l"));
    REQUIRE_NOTHROW(cmd.add_opt("l"));
    REQUIRE_NOTHROW(cmd.add_opt("-l"));
    REQUIRE_THROWS(cmd.add_arg("a"));
    REQUIRE_THROWS(cmd.add_opt("-a"));

    REQUIRE(cmd.execute(ops) == LISTIF);

    REQUIRE_NOTHROW(cmd2.add_opt("-s"));
    REQUIRE_THROWS(cmd2.add_opt("-s"));
    REQUIRE_NOTHROW(cmd2.add_arg("1"));
    REQUIRE_THROWS(cmd2.add_arg("2"));

    REQUIRE(cmd2.execute(ops) == SETIF);
  }
}

TEST_CASE("parser") {
  ControllerOperation ops(nullptr);
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
     std::make_tuple("start", 0, START),
     std::make_tuple("start -a", 1, 0),
     std::make_tuple("start a", 1, 0),
     std::make_tuple("stop", 0, STOP),
     std::make_tuple("stop -a", 1, 0),
     std::make_tuple("stop a", 1, 0),
     std::make_tuple("pause", 0, PAUSE),
     std::make_tuple("pause -a", 1, 0),
     std::make_tuple("pause a", 1, 0),
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

      REQUIRE(cmd->execute(ops) == std::get<2>(a));
    }
    else {
      REQUIRE(parser.get_cmd() == nullptr);
    }
  }
}

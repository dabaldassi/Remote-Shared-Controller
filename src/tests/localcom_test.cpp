#include <rsclocal_com.hpp>
#include <message.hpp>

#include "catch/catch.hpp"


TEST_CASE("Message") {
  using namespace rsclocalcom;
  
  SECTION("ACK") {
    std::stringstream ss;
    Message m(Message::ACK);

    REQUIRE_THROWS(m.add_arg("arg"));
    
    REQUIRE_NOTHROW(m.get(ss));
    REQUIRE(ss.str() == "ACK");
  }

  SECTION("GETLIST") {
    std::stringstream ss;
    Message m(Message::GETLIST);

    REQUIRE_NOTHROW(m.get(ss));
    REQUIRE(ss.str() == "GETLIST");
  }

  SECTION("SETLIST") {
    std::stringstream ss;
    Message m(Message::SETLIST);

    REQUIRE_NOTHROW(m.get(ss));
    REQUIRE(ss.str() == "SETLIST");
  }

  SECTION("IF") {
    std::stringstream ss;
    Message m(Message::IF);

    REQUIRE_THROWS(m.get(ss));
    
    REQUIRE_NOTHROW(m.add_arg(1));
    REQUIRE_NOTHROW(m.get(ss));
    REQUIRE(ss.str() == "IF 1");

    REQUIRE_THROWS(m.add_arg(2));

    m.reset(Message::IF);
    std::stringstream().swap(ss);
    REQUIRE_NOTHROW(m.add_arg("2"));
    REQUIRE_NOTHROW(m.get(ss));
    REQUIRE(ss.str() == "IF 2");
    
  }

  SECTION("NA") {
    std::stringstream ss;
    Message m;
    
    REQUIRE_THROWS(m.get(ss));
    REQUIRE_THROWS(m.add_arg(1));

    ss.str("GETLIST");
    REQUIRE_NOTHROW(m.set(ss));
    std::stringstream().swap(ss);
    REQUIRE_NOTHROW(m.get(ss));
    REQUIRE(ss.str() == "GETLIST");

    ss.clear();
    ss.str("IF 1");
    REQUIRE_NOTHROW(m.set(ss));
    std::stringstream().swap(ss);
    REQUIRE_NOTHROW(m.get(ss));
    REQUIRE(ss.str() == "IF 1");

    ss.clear();
    ss.str("IF");
    REQUIRE_THROWS(m.set(ss));
  }
}

TEST_CASE("Com") {
  using namespace rsclocalcom;

  std::stringstream ss;
  RSCLocalCom       com;
  Message           msg(Message::GETLIST);

  com.send_to(RSCLocalCom::Contact::CORE,msg);
  msg.reset();
  com.read_from(RSCLocalCom::Contact::CLIENT,msg);

  REQUIRE_NOTHROW(msg.get(ss));
  REQUIRE(ss.str() == "GETLIST");

  std::stringstream().swap(ss);
  msg.reset(Message::ACK);

  com.send_to(RSCLocalCom::Contact::CLIENT,msg);
  msg.reset();
  com.read_from(RSCLocalCom::Contact::CORE,msg);

  REQUIRE_NOTHROW(msg.get(ss));
  REQUIRE(ss.str() == "ACK");
}

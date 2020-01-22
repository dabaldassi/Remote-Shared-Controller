#include "catch/catch.hpp"

#include <iostream>
#include <fstream>
#include <pc_list.hpp>

TEST_CASE("PC") {
  using namespace rscutil;
  PC pc1 { 1, false, "pc", { 5,0x23,0,0,0,0 }, {10,0} ,{8,5} },
    pc2 { 1, false, "pc", { 5,0x23,0,0,0,0 }, {10,0} ,{8,5} };

  REQUIRE(pc1 == pc2);

  pc2.id = 2;

  REQUIRE(pc1 != pc2);

  std::ofstream ofs("/tmp/pc_list");

  REQUIRE(ofs.is_open());
  pc1.save(ofs);
  ofs.close();

  std::ifstream ifs("/tmp/pc_list");
  
  REQUIRE(ifs.is_open());
  PC pc;
  pc.load(ifs);
  ifs.close();

  REQUIRE(pc == pc1);
}

TEST_CASE("PC List") {
  using namespace rscutil;
  PC pc1 { 1, true, "pc1", { 0,0,0,0,0,0 }, {1920,1080} ,{0,0} };
  PC pc2 { 2, false, "pc2", { 5,0x23,0x45,0x74,0x12,0x01 }, {1024,720} ,{8,5} };
  PC pc3 { 3, false, "pc3", { 5,0x23,0x54,0x47,0x47,0x12 }, {640,480} ,{84,50} };
  PC pc4 { 4, false, "pc4", { 0x04,0x43,0xff,0x47,0x47,0x12 }, {640,480} ,{84,50} };

  PCList list;

  REQUIRE_THROWS(list.add(pc1, pc2.id));
  
  list.add(pc1);
  list.add(pc2);

  REQUIRE_NOTHROW(list.add(pc3, pc2.id));
  REQUIRE_NOTHROW(list.add(pc4, pc1.id));

  REQUIRE_THROWS(list.add(pc4, 456));

  REQUIRE(pc4 == list.get_current());
  list.next_pc();
  REQUIRE(pc1 == list.get_current());
  list.next_pc();
  REQUIRE(pc3 == list.get_current());
  list.next_pc();
  REQUIRE(pc2 == list.get_current());
  list.next_pc();
  REQUIRE(list.is_circular());
  REQUIRE(pc4 == list.get_current());
  list.previous_pc();
  REQUIRE(pc2 == list.get_current());
  list.previous_pc();
  REQUIRE(pc3 == list.get_current());

  list.next_pc();
  list.set_circular(false);
  REQUIRE_FALSE(list.is_circular());
  list.next_pc();

  REQUIRE(pc2 == list.get_current());
  REQUIRE(pc1 == list.get_local());

  REQUIRE_NOTHROW(list.save("/tmp/pc_list_save"));

  PCList list2;

  REQUIRE_NOTHROW(list2.load("/tmp/pc_list_save"));
  
  REQUIRE(pc4 == list2.get_current());
  list2.next_pc();
  REQUIRE(pc1 == list2.get_current());
  list2.next_pc();
  REQUIRE(pc3 == list2.get_current());
  list2.next_pc();
  REQUIRE(pc2 == list2.get_current());
}

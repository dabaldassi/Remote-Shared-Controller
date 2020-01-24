#include "catch/catch.hpp"

#include <iostream>
#include <fstream>

#include <pc_list.hpp>
#include <combo.hpp>
#include <config.hpp>
#include <controller.h>

#ifndef NO_CURSOR
#include <cursor.h>
#endif

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

TEST_CASE("Combo") {
  using namespace rscutil;
  
  SECTION("Shortcut") {
    std::string   name("combotest"), description("This is a combotest");
    ComboShortcut combo(name, description);
    bool          action = false;
    
    combo.add_shortcut(KEY_LEFTCTRL, 1);
    combo.add_shortcut(KEY_R, 1);
    combo.add_shortcut(KEY_RIGHT, 1);
    combo.set_action([&action](Combo *) { action = true; });

    auto test = [&name, &description, &action](ComboShortcut& cs) {
      REQUIRE(cs.get_name() == name);
      REQUIRE(cs.get_description() == description);
      REQUIRE(cs.to_string() == "LCTRL-R-->");
      
      REQUIRE_FALSE(cs.update(KEY_LEFTCTRL,1));
      REQUIRE_FALSE(cs.update(KEY_R,1));
      REQUIRE(cs.update(KEY_RIGHT,1));
      REQUIRE(action);
      REQUIRE(cs.get_way() == Combo::Way::RIGHT);

      action = false;

      REQUIRE_FALSE(cs.update(KEY_LEFTCTRL,1));
      REQUIRE_FALSE(cs.update(KEY_LEFTCTRL,1));
      REQUIRE_FALSE(cs.update(KEY_R,1));
      REQUIRE_FALSE(cs.update(KEY_RIGHT,1));
      REQUIRE_FALSE(action);

      REQUIRE_FALSE(cs.update(KEY_LEFTCTRL,1));
      REQUIRE_FALSE(cs.update(KEY_R,1));
      REQUIRE(cs.update(KEY_RIGHT,1));
      REQUIRE(action);

      action = false;
    };

    test(combo);

    constexpr char file_name[] = "/tmp/combo";
    
    std::ofstream ofs(file_name);

    REQUIRE(ofs.is_open());
    combo.save(ofs);
    ofs.close();
    
    ComboShortcut combo_load("comboload","descload");
    combo_load.set_action([&action](Combo *) { action = true; });

    std::ifstream ifs(file_name);

    REQUIRE(ifs.is_open());
    combo_load.load(ifs);
    ifs.close();

    test(combo_load);

    combo_load.release_for_all();

    REQUIRE_FALSE(combo_load.update(KEY_LEFTCTRL,1));
    REQUIRE_FALSE(combo_load.update(KEY_R,1));
    REQUIRE_FALSE(combo_load.update(KEY_RIGHT,1));
    REQUIRE_FALSE(combo_load.update(KEY_LEFTCTRL,0));
    REQUIRE_FALSE(combo_load.update(KEY_R,0));
    REQUIRE(combo_load.update(KEY_RIGHT,0));
    REQUIRE(action);
    REQUIRE(combo_load.to_string() == "LCTRL-R-->-(R)*-(R)*-(R)*");
  }
  
#ifndef NO_CURSOR
  SECTION("Mouse") {
    REQUIRE_THROWS(ComboMouse(100,100, nullptr));
    
    CursorInfo * cursor = open_cursor_info();
    ComboMouse   combo(100,100,cursor);
    bool         action = false;

    combo.set_action([&action](Combo*) { action = true; });
    
    cursor->pos_x = 50;
    cursor->pos_y = 50;
    set_cursor_position(cursor);
    REQUIRE_FALSE(combo.update(0,0));
    REQUIRE(combo.get_way() == Combo::Way::NONE);

    cursor->pos_x = 0;
    cursor->pos_y = 50;
    set_cursor_position(cursor);
    REQUIRE(combo.update(0,0));
    REQUIRE(combo.get_way() == Combo::Way::LEFT);
    REQUIRE(action);
    
    action = false;
    cursor->pos_x = 100;
    cursor->pos_y = 50;
    set_cursor_position(cursor);
    REQUIRE(combo.update(0,0));
    REQUIRE(combo.get_way() == Combo::Way::RIGHT);
    REQUIRE(action);

    cursor->pos_x = 50;
    cursor->pos_y = 50;
    set_cursor_position(cursor);
    REQUIRE_FALSE(combo.update(0,0));
    REQUIRE(combo.get_way() == Combo::Way::NONE);
    
    close_cursor_info(cursor);
  }
#endif
  
  SECTION("ShortcutList") {
    ComboShortcut::ComboShortcutList combolist;

    std::vector<std::pair<std::string, std::string>> nd = {
      { "combotest1", "descriptiontest1" },
      { "combotest2", "descriptiontest2" },
      { "combotest3", "descriptiontest3" },
    };

    for(size_t i = 0; i < nd.size(); ++i) {
      ComboShortcut combo(nd[i].first, nd[i].second);
      
      combo.add_shortcut(i, 1);
      combo.add_shortcut(i+1, 1);
      
      combo.set_action([](Combo *) {});

      combolist.push_back(std::move(combo));
    }

    auto test = [&nd](ComboShortcut::ComboShortcutList& list) {
      size_t i = 0;
      
      for(auto& cs : list) {
	REQUIRE(cs.get_name() == nd[i].first);
	REQUIRE(cs.get_description() == nd[i].second);
      
	REQUIRE_FALSE(cs.update(i,1));
	REQUIRE(cs.update(i+1,1));
	REQUIRE(cs.get_way() == Combo::Way::RIGHT);
	
	++i;
      }
    };

    test(combolist);

    ComboShortcut::save(combolist);
    
    ComboShortcut::ComboShortcutList loadlist;
    ComboShortcut::load(loadlist);

    for(auto& a: loadlist) a.set_action([](Combo*){});
    
    test(loadlist);
  }

  std::remove(RSC_SHORTCUT_SAVE);
}

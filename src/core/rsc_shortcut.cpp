#include <rsc.hpp>
#include <controller.h>

void RSC::save_shortcut() const
{
  using rscutil::ComboShortcut;
  ComboShortcut::ComboShortcutList list;
  
  for(const auto& a: _shortcut) {
    if(a->get_type() == ComboShortcut::TYPE) {
      list.push_back(*static_cast<ComboShortcut*>(a.get()));
    }
  }

  ComboShortcut::save(list);
}

void RSC::_send_release(rscutil::Combo* combo)
{
  using rscutil::ComboShortcut;
  
  auto * c = static_cast<ComboShortcut *>(combo);

  c->for_each([this](ComboShortcut::shortcut_t& s) {
      int code = std::get<0>(s);

      ControllerEvent e = { false, KEY, EV_KEY, KEY_RELEASED, 0};
      e.code = code;

      _send(e);
    });
}

void RSC::load_shortcut(bool reset)
{
  using rscutil::Combo;
  using rscutil::ComboShortcut;

  std::map<std::string, std::function<void(Combo*)>> _actions = {
    { "right", [this](Combo * combo) { _send_release(combo); _transit(combo->get_way()); } },
    { "left", [this](Combo * combo) { _send_release(combo); _transit(combo->get_way()); } },
    { "quit", [this](Combo*) { _run = false; } }
  };

  _shortcut.erase(std::remove_if(_shortcut.begin(),
				 _shortcut.end(),
				 [](auto&& a) { return a->get_type() == ComboShortcut::TYPE; }),
		  _shortcut.end());

  ComboShortcut::ComboShortcutList list;
  bool                             success = false;

  if(!reset) success = ComboShortcut::load(list);
  
  if(success) {
    for(auto& combo : list) {
      combo.set_action(_actions[combo.get_name()]);
      _shortcut.push_back(std::make_unique<ComboShortcut>(combo));
    }
  }
  else {
    // Default shortcut
    auto right = ComboShortcut::make_ptr("right", "Move to the next computer on the right");

    right->add_shortcut(KEY_LEFTCTRL, KEY_PRESSED);
    right->add_shortcut(KEY_R, KEY_PRESSED);
    right->add_shortcut(KEY_RIGHT, KEY_PRESSED);  
    right->release_for_all();
    right->set_action(_actions["right"]);

    auto left = ComboShortcut::make_ptr("left", "Move to the next computer on the left",
					Combo::Way::LEFT);

    left->add_shortcut(KEY_LEFTCTRL, KEY_PRESSED);
    left->add_shortcut(KEY_R, KEY_PRESSED);
    left->add_shortcut(KEY_LEFT, KEY_PRESSED);  
    left->release_for_all();
    left->set_action(_actions["left"]);
  
    auto quit = ComboShortcut::make_ptr("quit", "Quit the service", Combo::Way::NONE);

    for(int i = 0; i < 3; i++) {
      quit->add_shortcut(KEY_ESC, KEY_PRESSED, 200);
      quit->add_shortcut(KEY_ESC, KEY_RELEASED, 200);
    }
  
    quit->set_action(_actions["quit"]);

    _shortcut.push_back(std::move(right));
    _shortcut.push_back(std::move(left));
    _shortcut.push_back(std::move(quit));

    if(reset) save_shortcut();
  }
}

#pragma once
#include "app/main_menu.hpp"
#include "app/main_window.hpp"
#include "xtag/instance.hpp"

namespace xtag::gui {
class Controller : public Object {
  public:
	void initialize(Services const& services) final;
	void update() final;

  private:
	std::shared_ptr<Slot> m_slot{make_slot()};

	klib::Ptr<Instance> m_instance{};
	klib::Ptr<Services const> m_services{};

	MainMenu m_main_menu{};
	MainWindow m_main_window{};
};
} // namespace xtag::gui

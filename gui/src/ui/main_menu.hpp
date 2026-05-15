#pragma once
#include "ui/controller.hpp"

namespace xtag::gui::ui {
class MainMenu : public Object {
  public:
	explicit MainMenu(IController& controller) : m_controller(&controller) {}

	void update() final;

  private:
	klib::Ptr<IController> m_controller{};
};
} // namespace xtag::gui::ui

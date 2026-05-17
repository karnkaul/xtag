#pragma once
#include "ui/dispatch.hpp"

namespace xtag::gui::ui {
class MainMenu : public Object {
  public:
	explicit MainMenu(IDispatch& dispatch) : m_dispatch(&dispatch) {}

	void update() final;

  private:
	klib::Ptr<IDispatch> m_dispatch{};
};
} // namespace xtag::gui::ui

#pragma once
#include "ui/object.hpp"
#include <imgui.h>

namespace xtag::gui::ui {
class IController : public Object {
  public:
	virtual void shutdown() = 0;
	virtual void refresh_root_directory() = 0;

	virtual void open_test_modal() = 0;
};
} // namespace xtag::gui::ui

#pragma once
#include "app/object.hpp"
#include <imgui.h>

namespace xtag::gui {
class IController : public Object {
  public:
	virtual void shutdown() = 0;
	virtual void refresh_root_directory() = 0;
};
} // namespace xtag::gui

#pragma once
#include "app/object.hpp"

namespace xtag::gui {
class MainWindow : public Object {
  public:
	void update() final;
};
} // namespace xtag::gui

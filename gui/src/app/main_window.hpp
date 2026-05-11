#pragma once
#include "app/object.hpp"

namespace xtag::gui {
class MainWindow : public Object {
  public:
	void update() final;

  private:
	void update_model();
};
} // namespace xtag::gui

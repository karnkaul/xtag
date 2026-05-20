#pragma once
#include "klib/base_types.hpp"

namespace xtag::gui::ui {
class Object : public klib::Polymorphic {
  public:
	virtual void update() {}
};
} // namespace xtag::gui::ui

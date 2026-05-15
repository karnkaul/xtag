#pragma once
#include "klib/base_types.hpp"
#include "service/services.hpp"

namespace xtag::gui::ui {
class Object : public klib::Polymorphic {
  public:
	virtual void initialize(Services const& services) { m_services = &services; }
	virtual void update() {}

  protected:
	klib::Ptr<Services const> m_services{};
};
} // namespace xtag::gui::ui

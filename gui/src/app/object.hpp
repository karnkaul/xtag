#pragma once
#include "klib/base_types.hpp"
#include "service/services.hpp"

namespace xtag::gui {
class Object : public klib::Polymorphic {
  public:
	virtual void initialize(Services const& services);
	virtual void update() {}

  protected:
	klib::Ptr<Services const> m_services{};
};
} // namespace xtag::gui

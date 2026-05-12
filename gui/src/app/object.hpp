#pragma once
#include "klib/base_types.hpp"
#include "service/services.hpp"
#include "service/signals.hpp"

namespace xtag::gui {
class Object : public klib::Polymorphic {
  public:
	virtual void initialize(Services const& services);
	virtual void update() {}

  protected:
	klib::Ptr<Services const> m_services{};
	klib::Ptr<Signals> m_signals{};
};
} // namespace xtag::gui

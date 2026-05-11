#include "app/object.hpp"

namespace xtag::gui {
void Object::initialize(Services const& services) {
	m_services = &services;
	m_signals = &services.get<Signals>();
}
} // namespace xtag::gui

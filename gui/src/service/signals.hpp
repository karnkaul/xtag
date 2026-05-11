#pragma once
#include "service/signal.hpp"

namespace xtag::gui {
struct Signals {
	Signal<> shutdown{};
};
} // namespace xtag::gui

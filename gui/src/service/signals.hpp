#pragma once
#include "service/signal.hpp"

namespace xtag::gui {
struct Signals {
	Signal<> shutdown{};
	Signal<> refresh_root_directory{};
};
} // namespace xtag::gui

#pragma once
#include <chrono>

namespace xtag::gui {
using namespace std::chrono_literals;

using SteadyClock = std::chrono::steady_clock;
using SystemClock = std::chrono::system_clock;

using Seconds = std::chrono::duration<float>;

template <typename ClockT = SteadyClock>
[[nodiscard]] auto compute_dt(typename ClockT::time_point& out) -> Seconds {
	auto const now = ClockT::now();
	auto const ret = now - out;
	out = now;
	return ret;
}
} // namespace xtag::gui

#pragma once
#include <chrono>

namespace xtag::gui {
using namespace std::chrono_literals;

using SteadyClock = std::chrono::steady_clock;
using SystemClock = std::chrono::system_clock;

using Seconds = std::chrono::duration<float>;

[[nodiscard]] inline auto compute_dt(SteadyClock::time_point& out) -> Seconds {
	auto const now = SteadyClock::now();
	auto const ret = now - out;
	out = now;
	return ret;
}
} // namespace xtag::gui

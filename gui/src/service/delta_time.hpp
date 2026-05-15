#pragma once
#include "app/time.hpp"

namespace xtag::gui {
class DeltaTime {
  public:
	void update() { m_dt = compute_dt(m_frame_start); }

	void restart() { m_frame_start = SteadyClock::now(); }

	[[nodiscard]] auto get_dt() const -> Seconds { return m_dt; }

  private:
	SteadyClock::time_point m_frame_start{SteadyClock::now()};
	Seconds m_dt{};
};
} // namespace xtag::gui

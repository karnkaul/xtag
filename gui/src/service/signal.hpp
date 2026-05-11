#pragma once
#include "ksignal/ksignal.hpp"
#include <memory>

namespace xtag::gui {
using ksignal::Signal;

struct Slot {
	int id{};
};

[[nodiscard]] inline auto make_slot() {
	static auto s_id = 0;
	return std::make_shared<Slot>(Slot{.id = ++s_id});
}
} // namespace xtag::gui

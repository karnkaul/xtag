#pragma once
#include "xtag/types.hpp"

namespace xtag::gui::ui {
struct EntryData {
	[[nodiscard]] static auto read_from(Entry const& entry) -> EntryData const&;
	static void write_to(Entry& out, fs::path const& root);

	std::string relative_path{};
};
} // namespace xtag::gui::ui

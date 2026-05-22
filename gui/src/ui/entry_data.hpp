#pragma once
#include "xtag/types.hpp"

namespace xtag::gui::ui {
struct EntryData {
	Entry entry{};
	std::string relative_path{};
	std::string filename{};
};

struct EntryDataList {
	[[nodiscard]] static auto from(EntryList entry_list) -> EntryDataList;

	std::string path{};
	std::vector<EntryData> entries{};
};
} // namespace xtag::gui::ui

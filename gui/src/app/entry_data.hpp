#pragma once
#include <string>
#include <vector>

namespace xtag::gui {
struct EntryData {
	std::string tree_uri{};
	std::string inspect_uri{};
	std::string short_tags{};
	std::vector<std::string> all_tags{};
};
} // namespace xtag::gui

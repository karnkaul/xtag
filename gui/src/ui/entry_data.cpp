#include "ui/entry_data.hpp"

namespace xtag::gui::ui {
auto EntryDataList::from(EntryList entry_list) -> EntryDataList {
	entry_list.sort_entries();
	auto ret = EntryDataList{.path = entry_list.path.generic_string()};
	ret.entries.reserve(entry_list.entries.size());
	for (auto& entry : entry_list.entries) {
		auto entry_data = EntryData{.entry = std::move(entry)};
		entry_data.relative_path = fs::relative(entry_data.entry.path, entry_list.path).generic_string();
		entry_data.filename = entry_data.entry.path.filename().string();
		ret.entries.push_back(std::move(entry_data));
	}
	return ret;
}
} // namespace xtag::gui::ui

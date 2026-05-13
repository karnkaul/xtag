#include "app/entry_data.hpp"
#include "xtag/formatter.hpp"

namespace xtag::gui {
auto EntryData::from(Entry const& entry) -> EntryData {
	auto ret = EntryData{};

	if (entry.type == EntryType::Directory) {
		ret.inspect_filename = std::format("directory: {}", entry.path.filename().string()),
		ret.tree_filename = std::format("{}/", entry.path.filename().string());
	} else {
		ret.inspect_filename = std::format("file: {}", entry.path.filename().string());
		ret.tree_filename = entry.path.filename().string();
	}

	auto const formatter = Formatter{};
	for (auto const& tag : entry.tags) { ret.all_tags.push_back(formatter.format(tag)); }
	formatter.truncate_to(ret.short_tags, ret.all_tags, 3);

	return ret;
}

auto EntryData::to_model() const -> EntryModel {
	return EntryModel{
		.tree_filename = tree_filename,
		.inspect_filename = inspect_filename,
		.short_tags = short_tags,
		.all_tags = all_tags,
	};
}
} // namespace xtag::gui

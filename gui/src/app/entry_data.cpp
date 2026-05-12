#include "app/entry_data.hpp"
#include "xtag/util.hpp"

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
	static constexpr auto params_v = util::JoinParams{.max_count = 2};
	ret.short_tags = std::format("tags: {}", util::join_tags(entry.tags, params_v));
	ret.all_tags = util::as_strings(entry.tags);
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

#include "app/entry_data.hpp"
#include "xtag/formatter.hpp"

namespace xtag::gui {
auto EntryData::from(Entry const& entry, fs::path const& root) -> EntryData {
	auto ret = EntryData{};

	auto const filename = entry.path.filename().string();
	auto const uri = fs::relative(entry.path, root).generic_string();
	if (entry.type == EntryType::Directory) {
		ret.inspect_uri = std::format("directory: {}", filename);
		ret.tree_uri = std::format("{}/", uri);
	} else {
		ret.inspect_uri = std::format("file: {}", filename);
		ret.tree_uri = uri;
	}

	auto const formatter = Formatter{};
	for (auto const& tag : entry.tags) { ret.all_tags.push_back(formatter.format(tag)); }
	formatter.truncate_to(ret.short_tags, ret.all_tags, 3);

	return ret;
}

auto EntryData::to_model() const -> EntryModel {
	return EntryModel{
		.tree_uri = tree_uri,
		.inspect_uri = inspect_uri,
		.short_tags = short_tags,
		.all_tags = all_tags,
	};
}
} // namespace xtag::gui

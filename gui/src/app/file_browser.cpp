#include "app/file_browser.hpp"
#include <ranges>

namespace xtag::gui {
void FileBrowser::refresh(EntryList list) {
	m_list = std::move(list);
	m_view.clear();
	for (auto const& entry : m_list.entries) { m_view.push_back(&entry); }
	m_selected = {};
}

auto FileBrowser::select_entry(Entry const& entry) -> bool {
	auto const target = find_entry(entry.path);
	if (!target) { return false; }
	return on_select(*target);
}

void FileBrowser::apply_filter(std::string_view const allowlist, std::string_view const blocklist) {
	auto const allows = std::ranges::to<std::vector>(std::views::split(allowlist, ','));
	auto const blocks = std::ranges::to<std::vector>(std::views::split(blocklist, ','));

	static auto const is_match = [](auto const& set, std::string_view const text) {
		return std::ranges::any_of(set, [text](auto const& s) { return text.contains(std::string_view{s}); });
	};

	auto const should_include = [&](Entry const& entry) {
		auto const path = entry.path.generic_string();
		if (!blocks.empty() && is_match(blocks, path)) { return false; }
		return allows.empty() || is_match(allows, path);
	};

	m_view.clear();
	for (auto const& entry : m_list.entries) {
		if (!should_include(entry)) { continue; }
		m_view.emplace_back(&entry);
	}
}

auto FileBrowser::find_entry(fs::path const& path) const -> klib::Ptr<Entry const> {
	auto const it = std::ranges::find_if(m_list.entries, [&path](Entry const& e) { return e.path == path; });
	if (it == m_list.entries.end()) { return nullptr; }
	return &*it;
}

auto FileBrowser::on_select(Entry const& entry) -> bool {
	m_selected = &entry;
	return true;
}
} // namespace xtag::gui

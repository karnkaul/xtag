#include "app/file_list.hpp"
#include "xtag/panic.hpp"
#include <algorithm>
#include <ranges>

namespace xtag::gui {
FileList::FileList(EntryList list, int const page_limit) {
	if (list.entries.empty()) { throw Panic{"FileBrowser: EntryList is empty"}; }
	refresh(std::move(list));
	repaginate(page_limit);
}

auto FileList::get_current_page() const -> Page {
	if (m_page_number >= m_page_count) { return {}; }

	auto span = std::span{m_filtered};
	auto const limit = std::size_t(m_page_limit);
	auto const offset = std::size_t(m_page_number) * limit;
	KLIB_ASSERT(offset <= m_filtered.size());
	span = span.subspan(offset);
	auto const size = std::min(span.size(), limit);
	return Page{.entries = span.subspan(0, size), .offset_from_start = int(offset)};
}

void FileList::refresh(EntryList list) {
	auto const was_selected = m_selected.entry ? m_selected.entry->path : fs::path{};
	KLIB_ASSERT(!list.entries.empty());
	clear_pointers(list.entries.size());
	KLIB_ASSERT(m_filtered.empty() && m_path_map.empty() && !m_selected.entry);

	m_list = std::move(list); // henceforth entries in m_list must remain address-stable.

	for (auto const& [index, entry] : std::views::enumerate(m_list.entries)) {
		m_filtered.emplace_back(&entry);
		m_path_map.insert_or_assign(entry.path, EntryView{.entry = &entry, .index = int(index)});
	}

	if (!was_selected.empty()) { m_selected = find_entry(was_selected); }
	if (!m_selected.entry) { m_selected = EntryView{.entry = m_filtered.front(), .index = 0}; }

	repaginate(m_page_limit);
}

void FileList::repaginate(int const page_limit) {
	m_page_limit = std::clamp(page_limit, min_page_limit_v, max_page_limit_v);

	auto const filtered = int(m_filtered.size());
	m_page_count = filtered / m_page_limit;
	if (filtered % m_page_limit) { ++m_page_count; }

	KLIB_ASSERT(m_selected.index >= 0);

	auto const selected_page = (m_selected.index + 1) / m_page_limit;
	KLIB_ASSERT(selected_page <= m_page_count);
	m_page_number = selected_page;
}

auto FileList::select_entry(Entry const& entry) -> bool {
	auto const target = find_entry(entry.path);
	if (!target.entry) { return false; }
	m_selected = target;
	return true;
}

auto FileList::set_page_number(int const page_number) -> bool {
	if (page_number >= get_page_count()) { return false; }
	m_page_number = page_number;
	return true;
}

void FileList::apply_filter(std::string_view const allowlist, std::string_view const blocklist) {
	auto const allows = std::ranges::to<std::vector>(std::views::split(allowlist, ','));
	auto const blocks = std::ranges::to<std::vector>(std::views::split(blocklist, ','));

	static auto const is_match = [](auto const& set, std::string_view const text) {
		return std::ranges::any_of(set, [text](auto const& s) { return text.contains(std::string_view{s}); });
	};

	auto const should_include = [&](Entry const& entry) {
		auto const path = fs::relative(entry.path, m_list.path).generic_string();
		if (!blocks.empty() && is_match(blocks, path)) { return false; }
		return allows.empty() || is_match(allows, path);
	};

	m_filtered.clear();
	for (auto& entry : m_list.entries) {
		if (!should_include(entry)) { continue; }
		m_filtered.emplace_back(&entry);
	}
	repaginate(m_page_limit);
}

void FileList::clear_pointers(std::size_t const reserve) {
	m_selected = {};
	m_path_map.clear();
	m_filtered.clear();
	m_filtered.reserve(reserve);
	m_path_map.reserve(reserve);
}

auto FileList::find_entry(fs::path const& path) const -> EntryView {
	auto const it = m_path_map.find(path);
	if (it == m_path_map.end()) { return {}; }
	return it->second;
}
} // namespace xtag::gui

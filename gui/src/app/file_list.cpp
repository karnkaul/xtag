#include "app/file_list.hpp"
#include "xtag/panic.hpp"
#include "xtag/query.hpp"
#include <algorithm>
#include <ranges>

namespace xtag::gui {
FileList::FileList(std::shared_ptr<EntryList const> list, int const page_limit) {
	if (!list || list->entries.empty()) { throw Panic{"FileList: EntryList is empty"}; }
	refresh(std::move(list));
	repaginate(page_limit);
}

auto FileList::get_current_page() const -> Page {
	if (m_page_number >= m_page_count) { return {}; }

	auto const& entries = get_filtered();
	auto slice = std::span{entries};
	auto const limit = std::size_t(m_page_limit);
	auto const offset = std::size_t(m_page_number) * limit;
	KLIB_ASSERT(offset <= entries.size());
	slice = slice.subspan(offset);
	auto const size = std::min(slice.size(), limit);
	return Page{.entries = slice.subspan(0, size), .offset_from_start = int(offset)};
}

void FileList::refresh(std::shared_ptr<EntryList const> list, int new_page_limit) {
	KLIB_ASSERT(list);

	auto const was_selected = m_selected.entry ? m_selected.entry->path : fs::path{};
	KLIB_ASSERT(!list->entries.empty());
	clear_pointers(list->entries.size());
	KLIB_ASSERT(m_all.empty() && m_filtered.empty() && m_path_map.empty() && !m_selected.entry);

	m_list = std::move(list);

	for (auto const& [index, entry] : std::views::enumerate(m_list->entries)) {
		m_all.emplace_back(&entry);
		m_path_map.insert_or_assign(entry.path, EntryView{.entry = &entry, .index = int(index)});
	}

	if (!was_selected.empty()) { m_selected = find_entry(was_selected); }
	if (!m_selected.entry) { m_selected = EntryView{.entry = m_all.front(), .index = 0}; }

	if (new_page_limit <= 0) { new_page_limit = m_page_limit; }
	repaginate(new_page_limit);
}

void FileList::repaginate(int const page_limit) {
	m_page_limit = std::clamp(page_limit, min_page_limit_v, max_page_limit_v);

	auto const filtered = int(get_filtered().size());
	m_page_count = filtered / m_page_limit;
	if (filtered % m_page_limit) { ++m_page_count; }

	KLIB_ASSERT(m_selected.entry);
	m_page_number = page_number_for(*m_selected.entry);
}

auto FileList::select_entry(Entry const& entry) -> bool {
	auto const target = find_entry(entry.path);
	if (!target.entry) { return false; }
	m_selected = target;
	return true;
}

auto FileList::set_page_number(int const page_number) -> bool {
	if (page_number < 0 || page_number >= get_page_count()) { return false; }
	m_page_number = page_number;
	return true;
}

void FileList::filter_by_query(std::string_view const query) {
	m_filtered.clear();
	if (!query.empty()) {
		auto const expression = query::parse(query);
		for (auto const& entry : m_list->entries) {
			auto const filename = entry.path.filename().generic_string();
			if (!query.empty() && !expression.is_match(filename, entry.tags)) { continue; }
			m_filtered.emplace_back(&entry);
		}
	}

	KLIB_ASSERT(m_selected.entry);
	m_page_number = page_number_for(*m_selected.entry);
}

void FileList::clear_filter() { filter_by_query({}); }

void FileList::clear_pointers(std::size_t const reserve) {
	m_selected = {};
	m_list = {};
	m_path_map.clear();
	m_all.clear();
	m_filtered.clear();
	m_all.reserve(reserve);
	m_path_map.reserve(reserve);
}

auto FileList::find_entry(fs::path const& path) const -> EntryView {
	auto const it = m_path_map.find(path);
	if (it == m_path_map.end()) { return {}; }
	return it->second;
}

auto FileList::page_number_for(Entry const& entry) const -> int {
	auto const& entries = get_filtered();
	auto const it = std::ranges::find_if(entries, [&entry](klib::Ptr<Entry const> e) { return e == &entry; });
	if (it == entries.end()) { return 0; }

	auto const filtered_index = int(std::distance(entries.begin(), it));
	auto const selected_page = (filtered_index + 1) / m_page_limit;
	KLIB_ASSERT(selected_page <= m_page_count);
	return selected_page;
}
} // namespace xtag::gui

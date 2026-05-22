#include "ui/entry_book.hpp"
#include "xtag/panic.hpp"
#include "xtag/query.hpp"
#include <algorithm>
#include <ranges>

namespace xtag::gui::ui {
EntryBook::EntryBook(std::shared_ptr<EntryDataList const> list, int const page_limit) {
	if (!list || list->entries.empty()) { throw Panic{"FileList: EntryDataList is empty"}; }
	refresh(std::move(list));
	repaginate(page_limit);
}

auto EntryBook::get_current_page() const -> Page {
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

void EntryBook::refresh(std::shared_ptr<EntryDataList const> list, int new_page_limit) {
	KLIB_ASSERT(list);

	auto const was_selected = m_selected.entry_data ? m_selected.entry_data->entry.path : fs::path{};
	KLIB_ASSERT(!list->entries.empty());
	clear_pointers(list->entries.size());
	KLIB_ASSERT(m_all.empty() && m_filtered.empty() && m_path_map.empty() && !m_selected.entry_data);

	m_list = std::move(list);

	for (auto const& [index, entry_data] : std::views::enumerate(m_list->entries)) {
		m_all.emplace_back(&entry_data);
		m_path_map.insert_or_assign(entry_data.entry.path, EntryView{.entry_data = &entry_data, .index = int(index)});
	}

	if (!was_selected.empty()) { m_selected = find_entry(was_selected); }
	if (!m_selected.entry_data) { m_selected = EntryView{.entry_data = m_all.front(), .index = 0}; }

	if (new_page_limit <= 0) { new_page_limit = m_page_limit; }
	repaginate(new_page_limit);
}

void EntryBook::repaginate(int const page_limit) {
	m_page_limit = std::clamp(page_limit, min_page_limit_v, max_page_limit_v);

	auto const filtered = int(get_filtered().size());
	m_page_count = filtered / m_page_limit;
	if (filtered % m_page_limit) { ++m_page_count; }

	KLIB_ASSERT(m_selected.entry_data);
	m_page_number = page_number_for(*m_selected.entry_data);
}

auto EntryBook::select_entry(EntryData const& entry_data) -> bool {
	auto const target = find_entry(entry_data.entry.path);
	if (!target.entry_data) { return false; }
	m_selected = target;
	return true;
}

auto EntryBook::set_page_number(int const page_number) -> bool {
	if (page_number < 0 || page_number >= get_page_count()) { return false; }
	m_page_number = page_number;
	return true;
}

void EntryBook::filter_by_query(std::string_view const query) {
	m_filtered.clear();
	if (!query.empty()) {
		auto const expression = query::parse(query);
		for (auto const& entry_data : m_list->entries) {
			if (!query.empty() && !expression.is_match(entry_data.filename, entry_data.entry.tags)) { continue; }
			m_filtered.emplace_back(&entry_data);
		}
	}

	KLIB_ASSERT(m_selected.entry_data);
	m_page_number = page_number_for(*m_selected.entry_data);
}

void EntryBook::clear_filter() { filter_by_query({}); }

void EntryBook::clear_pointers(std::size_t const reserve) {
	m_selected = {};
	m_list = {};
	m_path_map.clear();
	m_all.clear();
	m_filtered.clear();
	m_all.reserve(reserve);
	m_path_map.reserve(reserve);
}

auto EntryBook::find_entry(fs::path const& path) const -> EntryView {
	auto const it = m_path_map.find(path);
	if (it == m_path_map.end()) { return {}; }
	return it->second;
}

auto EntryBook::page_number_for(EntryData const& entry) const -> int {
	auto const& entries = get_filtered();
	auto const it = std::ranges::find_if(entries, [&entry](klib::Ptr<EntryData const> e) { return e == &entry; });
	if (it == entries.end()) { return 0; }

	auto const filtered_index = int(std::distance(entries.begin(), it));
	auto const selected_page = (filtered_index + 1) / m_page_limit;
	KLIB_ASSERT(selected_page <= m_page_count);
	return selected_page;
}
} // namespace xtag::gui::ui

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
	KLIB_ASSERT(!list.entries.empty());
	m_list = std::move(list);
	m_filtered.clear();
	for (auto const& entry : m_list.entries) { m_filtered.emplace_back(&entry); }
	m_selected = m_filtered.front();
	repaginate(m_page_limit);
}

void FileList::repaginate(int const page_limit) {
	m_page_limit = std::clamp(page_limit, min_page_limit_v, max_page_limit_v);

	auto const filtered = int(m_filtered.size());
	m_page_count = filtered / m_page_limit;
	if (filtered % m_page_limit) { ++m_page_count; }

	m_page_number = 0;
}

auto FileList::select_entry(Entry const& entry) -> bool {
	auto const target = find_entry(entry.path);
	if (!target) { return false; }
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
	for (auto const& entry : m_list.entries) {
		if (!should_include(entry)) { continue; }
		m_filtered.emplace_back(&entry);
	}
	repaginate(m_page_limit);
}

auto FileList::find_entry(fs::path const& path) const -> klib::Ptr<Entry const> {
	auto const it = std::ranges::find_if(m_list.entries, [&path](Entry const& e) { return e.path == path; });
	if (it == m_list.entries.end()) { return nullptr; }
	return &*it;
}
} // namespace xtag::gui

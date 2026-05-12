#include "app/file_browser.hpp"

namespace xtag::gui {
auto FileBrowser::get_pwd() const -> Directory {
	return Directory{
		.entry = m_stack.empty() ? &get_root() : m_stack.back().get(),
		.relative_path = m_pwd_rel_path,
	};
}

void FileBrowser::refresh(Entry root) {
	m_root = std::move(root);
	m_root.sort_recursive();
	m_selected = {};
	m_stack.clear();
	refresh_data();
}

auto FileBrowser::set_selected(Entry const& entry) -> bool {
	auto const& pwd = get_pwd();
	if (&entry == pwd.entry) {
		m_selected = pwd.entry;
		return true;
	}

	auto const it = std::ranges::find_if(pwd.entry->subentries, [&entry](Entry const& e) { return e.path == entry.path; });
	if (it == pwd.entry->subentries.end()) { return false; }

	m_selected = &*it;
	return true;
}

auto FileBrowser::can_navigate_up() const -> bool { return !m_stack.empty(); }

void FileBrowser::navigate_up() {
	if (!can_navigate_up()) { return; }
	m_selected = {};
	m_stack.pop_back();
	refresh_data();
}

auto FileBrowser::navigate_to_selected() -> bool {
	auto const selected = get_selected();
	if (!selected || selected->type != EntryType::Directory) { return false; }

	if (selected == get_pwd().entry) { return true; } // already here.

	m_stack.push_back(m_selected);
	m_selected = {};
	refresh_data();
	return true;
}

void FileBrowser::navigate_to_root() {
	m_stack.clear();
	m_selected = {};
}

void FileBrowser::refresh_data() { m_pwd_rel_path = fs::relative(get_pwd().entry->path, get_root().path).generic_string(); }
} // namespace xtag::gui

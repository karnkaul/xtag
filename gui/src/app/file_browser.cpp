#include "app/file_browser.hpp"

namespace xtag::gui {
auto FileBrowser::get_pwd() const -> Directory {
	return Directory{
		.entry = m_stack.empty() ? &get_root() : m_stack.back().get(),
		.relative_path = m_pwd_rel_path,
	};
}

auto FileBrowser::has_parent() const -> bool { return !m_stack.empty(); }

auto FileBrowser::get_parent() const -> klib::Ptr<Entry const> {
	if (m_stack.empty()) { return nullptr; }
	if (m_stack.size() == 1) { return &get_root(); }
	return m_stack.at(m_stack.size() - 2);
}

void FileBrowser::refresh(Entry root) {
	m_root = std::move(root);
	m_root.sort_recursive();
	open_root();
}

auto FileBrowser::select_subentry(Entry const& entry) -> bool {
	auto const target = find_subentry(entry.path);
	if (!target) { return false; }
	return on_select(*target);
}

void FileBrowser::select_pwd() { m_selected = get_pwd().entry; }

auto FileBrowser::select_parent() -> bool {
	if (!has_parent()) { return false; }
	return on_select(*get_parent());
}

auto FileBrowser::open_parent() -> bool {
	if (!has_parent()) { return false; }
	m_stack.pop_back();
	return on_pwd_changed();
}

auto FileBrowser::open_selected() -> bool {
	auto const selected = get_selected();
	if (!selected || selected->type != EntryType::Directory) { return false; }

	if (selected == get_pwd().entry) { return true; } // already here.

	m_stack.push_back(selected);
	return on_pwd_changed();
}

void FileBrowser::open_root() {
	m_stack.clear();
	on_pwd_changed();
}

auto FileBrowser::find_subentry(fs::path const& path) const -> klib::Ptr<Entry const> {
	auto const& pwd = get_pwd();
	auto const it = std::ranges::find_if(pwd.entry->subentries, [&path](Entry const& e) { return e.path == path; });
	if (it == pwd.entry->subentries.end()) { return nullptr; }
	return &*it;
}

auto FileBrowser::on_pwd_changed() -> bool {
	auto const& pwd = get_pwd();
	m_pwd_rel_path = fs::relative(pwd.entry->path, get_root().path).generic_string();
	return on_select(*pwd.entry);
}

auto FileBrowser::on_select(Entry const& entry) -> bool {
	m_selected = &entry;
	return true;
}
} // namespace xtag::gui

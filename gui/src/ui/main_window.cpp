#include "ui/main_window.hpp"
#include "app/log.hpp"
#include "klib/string/c_string.hpp"
#include <imgui.h>
#include <ranges>

namespace xtag::gui::ui {
namespace {
struct EntryData {
	std::string tree_uri{};
	std::string inspect_uri{};
	std::vector<std::string> all_tags{};
};

auto to_data(Entry const& entry, fs::path const& root) -> EntryData {
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

	return ret;
}

void populate_data(EntryList& list) {
	for (auto& entry : list.entries) { entry.custom_payload = to_data(entry, list.path); }
}

constexpr auto tag_viewer_label_v = klib::CString{"tag_viewer"};

constexpr auto get_tag_color(TagType const type) {
	switch (type) {
	case TagType::Primary: return ImVec4{0.8f, 0.8f, 0.0f, 1.0f};
	case TagType::Inherited: return ImVec4{0.0f, 0.8f, 1.0f, 1.0f};
	default: return ImVec4{0.5f, 0.5f, 0.5f, 1.0f};
	}
}

void tag_text_colored(ScanTag const& tag) {
	auto const color = get_tag_color(tag.type);
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	ImGui::TextColored(color, "%s", tag.value.data());
}

void update_inspector(Entry const& selected) {
	static constexpr auto max_tags_v{3};
	auto const& data = std::any_cast<EntryData const&>(selected.custom_payload);
	ImGui::TextUnformatted(data.inspect_uri.c_str());
	for (auto const& [index, tag] : std::views::enumerate(selected.tags)) {
		if (index > 0) {
			ImGui::SameLine();
			ImGui::TextUnformatted("|");
			ImGui::SameLine();
		}
		if (index >= max_tags_v) {
			ImGui::TextUnformatted("...");
			break;
		}
		tag_text_colored(tag);
	}
}

void update_tag_viewer(Entry const& entry) {
	if (!ImGui::BeginPopup(tag_viewer_label_v.c_str())) { return; }
	if (ImGui::BeginListBox("tags", {200.0f, 0.0f})) {
		for (auto const& tag : entry.tags) { tag_text_colored(tag); }
		ImGui::EndListBox();
	}
	ImGui::EndPopup();
}
} // namespace

void MainWindow::update() {
	if (!m_file_browser) {
		ImGui::TextUnformatted("drag a directory here to begin");
		return;
	}

	ImGui::TextUnformatted(m_root_directory.c_str());

	auto const& selected = m_file_browser->list.get_selected();

	auto open_tag_viewer = false;
	auto refresh_root_directory = false;

	if (ImGui::BeginTable("top", 2, ImGuiTableFlags_Borders)) {
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		update_inspector(selected);
		if (ImGui::Button("view tags")) { open_tag_viewer = true; }

		ImGui::TableNextColumn();
		scan_data.update();
		if (ImGui::Button("refresh")) { refresh_root_directory = true; }

		ImGui::EndTable();
	}

	m_file_browser->update_filter();
	m_file_browser->update_pagination();

	if (open_tag_viewer) {
		open_tag_viewer = false;
		ImGui::OpenPopup(tag_viewer_label_v.c_str());
	}
	update_tag_viewer(selected);

	if (ImGui::BeginChild("list", {}, ImGuiChildFlags_Borders)) {
		update_current_page();
		ImGui::EndChild();
	}

	if (refresh_root_directory) {
		refresh_root_directory = false;
		m_dispatch->refresh_root_directory();
	}
}

void MainWindow::set_list(EntryList list) {
	list.sort_entries();
	m_root_directory = list.path.generic_string();
	populate_data(list);
	if (!m_file_browser) {
		m_file_browser.emplace(std::move(list));
	} else {
		m_file_browser->list.refresh(std::move(list));
	}
	log.debug("Directory loaded successfully: '{}'", m_root_directory);
}

void MainWindow::update_current_page() {
	auto const page = m_file_browser->list.get_current_page();
	m_file_browser->update_number_width(page);
	for (auto const& [index, entry] : std::views::enumerate(page.entries)) {
		auto const& data = std::any_cast<EntryData const&>(entry->custom_payload);
		auto const number = int(index) + page.offset_from_start + 1;
		m_file_browser->update_entry(number, *entry, data.tree_uri);
	}
}
} // namespace xtag::gui::ui

#include "app/main_window.hpp"
#include "app/log.hpp"
#include "klib/debug/assert.hpp"
#include "klib/string/c_string.hpp"
#include <imgui.h>

namespace xtag::gui {
namespace {
void populate_data(Entry& entry) {
	entry.custom_payload = EntryData::from(entry);
	for (auto& subentry : entry.subentries) { populate_data(subentry); }
}

[[nodiscard]] auto to_model(klib::Ptr<Entry const> selected) -> EntryModel {
	if (!selected) { return {}; }

	auto const* data = std::any_cast<EntryData>(&selected->custom_payload);
	if (!data) { return {}; }

	return data->to_model();
}

constexpr auto tag_viewer_label_v = klib::CString{"tag_viewer"};

void update_tag_viewer(EntryModel const& entry) {
	if (!ImGui::BeginPopup(tag_viewer_label_v.c_str())) { return; }
	if (ImGui::BeginListBox("tags", {200.0f, 0.0f})) {
		for (auto const& tag : entry.all_tags) { ImGui::TextUnformatted(tag.c_str()); }
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

	auto const entry = to_model(m_file_browser->get_selected());

	if (ImGui::BeginTable("controls", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		update_controls();

		ImGui::TableNextColumn();
		update_inspector(entry);

		ImGui::EndTable();
	}

	update_navigation();
	if (m_open_tag_viewer) {
		m_open_tag_viewer = false;
		ImGui::OpenPopup(tag_viewer_label_v.c_str());
	}
	update_tag_viewer(entry);

	if (ImGui::BeginChild("list", {}, ImGuiChildFlags_Borders)) {
		update_browser();
		ImGui::EndChild();
	}
}

void MainWindow::refresh_root_directory(Entry directory) {
	populate_data(directory);
	m_root_directory = fs::canonical(directory.path).generic_string();
	if (!m_file_browser) {
		m_file_browser.emplace(std::move(directory));
	} else {
		m_file_browser->refresh(std::move(directory));
	}
	log.debug("Directory loaded successfully: '{}'", m_root_directory);
}

void MainWindow::update_controls() {
	ImGui::Checkbox("include files", &scan_data.include_files);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.0f);
	ImGui::DragInt("scan depth", &scan_data.depth, 1.0f, 0, 100);

	if (ImGui::Button("refresh")) { m_signals->refresh_root_directory.dispatch(); }
}

void MainWindow::update_navigation() {
	KLIB_ASSERT(m_file_browser);

	ImGui::TextUnformatted(m_root_directory.c_str());
	ImGui::SameLine();
	ImGui::TextUnformatted(m_file_browser->get_pwd().relative_path.c_str());

	ImGui::BeginDisabled(!m_file_browser->has_parent());
	if (ImGui::Button("up")) { m_file_browser->open_parent(); }
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("root")) { m_file_browser->open_root(); }
	ImGui::SameLine();
	ImGui::SetNextItemWidth(150.0f);
	ImGui::InputText("filter", m_filename_filter.data(), m_filename_filter.size());
}

void MainWindow::update_inspector(EntryModel const& entry) {
	ImGui::TextUnformatted(entry.inspect_filename.c_str());
	ImGui::TextUnformatted(entry.short_tags.c_str());

	if (ImGui::Button("view tags")) { m_open_tag_viewer = true; }
}

void MainWindow::update_browser() {
	KLIB_ASSERT(m_file_browser);

	enum class Target : std::int8_t { None, Selected, Parent };

	static auto const item_double_clicked = [] { return ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left); };

	auto target = Target::None;
	auto const selected = m_file_browser->get_selected();

	auto const update_subentry = [&](klib::CString const filename, Entry const& subentry) {
		if (ImGui::Selectable(filename.c_str(), &subentry == selected)) { m_file_browser->select_subentry(subentry); }
		if (subentry.type == EntryType::Directory && item_double_clicked()) { target = Target::Selected; }
	};

	if (m_file_browser->has_parent()) {
		if (ImGui::Selectable("..", selected == m_file_browser->get_parent())) { m_file_browser->select_parent(); }
		if (item_double_clicked()) { target = Target::Parent; }
	}

	auto const& pwd = m_file_browser->get_pwd();
	if (ImGui::Selectable(".", selected == pwd.entry)) { m_file_browser->select_pwd(); }

	std::string_view const filename_filter = m_filename_filter.data();
	auto const is_filtered = [&](klib::CString const tree_filename) { return !filename_filter.empty() && !tree_filename.as_view().contains(filename_filter); };

	for (auto const& subentry : pwd.entry->subentries) {
		auto const entry_model = to_model(&subentry);
		if (is_filtered(entry_model.tree_filename)) { continue; }
		update_subentry(entry_model.tree_filename, subentry);
	}

	switch (target) {
	default:
	case Target::None: break;
	case Target::Selected: m_file_browser->open_selected(); break;
	case Target::Parent: m_file_browser->open_parent(); break;
	}
}
} // namespace xtag::gui

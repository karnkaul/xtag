#include "ui/widget/file_browser.hpp"
#include "klib/string/fixed_string.hpp"

namespace xtag::gui::ui::widget {
void FileBrowser::update_pagination() {
	auto const filtered_count = int(list.get_filtered().size());
	auto const page_number = list.get_page_number();

	auto const page = list.get_current_page();
	auto const page_count = list.get_page_count();
	auto const entry_count = int(page.entries.size());
	auto const start_number = page.offset_from_start + 1;
	auto const end_number = start_number + entry_count - 1;

	ImGui::BeginDisabled(page_number == 0);
	if (ImGui::Button("<<")) { list.set_page_number(page_number - 1); }
	ImGui::EndDisabled();
	ImGui::SameLine();

	ImGui::BeginDisabled(page_count == 0);
	ImGui::SetNextItemWidth(50.0f);
	if (ImGui::BeginCombo("##page", klib::FixedString{"{}", page_number}.c_str())) {
		for (int number = 0; number < page_count; ++number) {
			if (ImGui::Selectable(klib::FixedString{"{}", number}.c_str(), number == page_number)) { list.set_page_number(number); }
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(page_number + 1 == page_count);
	if (ImGui::Button(">>")) { list.set_page_number(page_number + 1); }
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::TextUnformatted(klib::FixedString{"{}-{} / {}", start_number, end_number, filtered_count}.c_str());

	ImGui::SameLine();
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::BeginCombo("items per page", klib::FixedString<>{"{}", list.get_page_limit()}.c_str())) {
		auto const page_limit = [this](int const limit) {
			if (ImGui::Selectable(klib::FixedString{"{}", limit}.c_str(), list.get_page_limit() == limit)) { list.repaginate(limit); }
		};
		for (auto const limit : page_limits_v) { page_limit(limit); }
		ImGui::EndCombo();
	}
}

auto FileBrowser::update_filter(std::string_view& out_query) -> bool {
	ImGui::SetNextItemWidth(150.0f);
	m_query_input.update("query");
	out_query = m_query_input.as_view();
	ImGui::SameLine();
	return ImGui::Button("search");
}

void FileBrowser::update_number_width(FileList::Page const& current_page) {
	m_number_width = 1;
	for (auto count = current_page.offset_from_start + int(current_page.entries.size()); count >= 10; count /= 10) { ++m_number_width; }
}

void FileBrowser::update_entry(int const number, Entry const& entry, klib::CString const path) {
	ImGui::TextUnformatted(klib::FixedString{"{: >{}}.", number, m_number_width}.c_str());
	ImGui::SameLine();
	if (ImGui::Selectable(path.c_str(), &entry == &list.get_selected())) { list.select_entry(entry); }
}
} // namespace xtag::gui::ui::widget

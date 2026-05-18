#include "ui/widget/tag_editor.hpp"
#include "klib/string/fixed_string.hpp"
#include <imgui.h>
#include <ranges>

namespace xtag::gui::ui::widget {
void TagEditor::set_should_open(Entry const& selected) {
	m_replacement.clear();
	m_input.clear();
	m_list.clear(selected.tags.size());
	m_dirty = false;
	for (auto const& tag : selected.tags) { m_list.ins.push_back(In{.tag = tag}); }

	m_should_open = true;
}

void TagEditor::update_short_tags(std::span<ScanTag const> tags) {
	static constexpr auto max_tags_v{3};
	for (auto const& [index, tag] : std::views::enumerate(tags)) {
		if (index > 0) {
			ImGui::SameLine();
			ImGui::TextUnformatted("|");
			ImGui::SameLine();
		}
		if (index >= max_tags_v) {
			ImGui::TextUnformatted("...");
			break;
		}
		auto const color = widget::get_tag_color(tag.type);
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
		ImGui::TextColored(color, "%s", tag.value.data());
	}
}

auto TagEditor::update() -> bool {
	if (m_should_open) {
		m_should_open = false;
		m_input.clear();
		ImGui::OpenPopup(label_v.c_str());
	}

	if (!ImGui::BeginPopup(label_v.c_str())) { return false; }

	update_header();
	update_tags();
	ImGui::Separator();
	update_new_tag();
	ImGui::Separator();
	auto ret = update_buttons();

	ImGui::EndPopup();

	if (ret) { ret = compute_replacement(); }
	return ret;
}

void TagEditor::update_header() const {
	if (m_dirty) {
		ImGui::TextUnformatted("*Tags");
	} else {
		ImGui::TextUnformatted("Tags");
	}
}

void TagEditor::update_tags() {
	if (!ImGui::BeginListBox("tags", {200.0f, 0.0f})) { return; }

	for (auto [index, tag] : std::views::enumerate(m_list.ins)) {
		ImGui::BeginDisabled(tag.tag.type != TagType::Primary);
		m_dirty |= ImGui::Checkbox(klib::FixedString{"##keep_in{}", index}.c_str(), &tag.should_keep);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, get_tag_color(tag.tag.type));
		ImGui::TextUnformatted(tag.tag.value.data());
		ImGui::PopStyleColor();
	}

	for (auto [index, tag] : std::views::enumerate(m_list.outs)) {
		auto keep = true;
		ImGui::Checkbox(klib::FixedString{"##keep_out{}", index}.c_str(), &keep);
		ImGui::SameLine();
		ImGui::TextUnformatted(tag.c_str());
		if (!keep) {
			m_list.outs.erase(m_list.outs.begin() + std::ptrdiff_t(index));
			break;
		}
	}

	ImGui::EndListBox();
}

void TagEditor::update_new_tag() {
	ImGui::SetNextItemWidth(60.0f);
	m_input.update("new");
	ImGui::SameLine();
	auto const new_tag = m_input.as_view();
	ImGui::BeginDisabled(new_tag.empty());
	if (ImGui::Button("Add")) {
		m_list.outs.emplace_back(new_tag);
		m_input.clear();
		m_dirty = true;
	}
	ImGui::EndDisabled();
}

auto TagEditor::update_buttons() const -> bool {
	auto ret = false;
	ImGui::BeginDisabled(!m_dirty);
	if (ImGui::Button("Save Changes")) {
		ret = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
	return ret;
}

auto TagEditor::compute_replacement() -> bool {
	m_replacement.clear();
	if (m_list.outs.empty() && std::ranges::all_of(m_list.ins, [](In const& in) { return in.should_keep; })) { return false; }

	for (auto const& in : m_list.ins) {
		if (!in.should_keep || in.tag.type != TagType::Primary) { continue; }
		m_replacement.emplace_back(in.tag.value);
	}
	std::ranges::move(m_list.outs, std::back_inserter(m_replacement));
	m_list.clear();
	return true;
}

void TagEditor::List::clear(std::size_t const ins_reserve) {
	ins.clear();
	outs.clear();
	ins.reserve(ins_reserve);
}
} // namespace xtag::gui::ui::widget

#include "ui/widget/tag_editor.hpp"
#include "klib/string/fixed_string.hpp"
#include <imgui.h>
#include <ranges>

namespace xtag::gui::ui::widget {
namespace {
constexpr auto get_tag_color(TagType const type) -> ImVec4 {
	switch (type) {
	case TagType::Primary: return ImVec4{0.8f, 0.8f, 0.0f, 1.0f};
	case TagType::Inherited: return ImVec4{0.0f, 0.8f, 1.0f, 1.0f};
	default: return ImVec4{0.5f, 0.5f, 0.5f, 1.0f};
	}
}
} // namespace

void TagEditor::extract_tags(Entry const& selected) {
	m_input.clear();
	m_tags.clear();
	m_tags.reserve(selected.tags.size());
	for (auto const& tag : selected.tags) { m_tags.push_back(Tag{.in = tag}); }
	m_dirty = false;
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
		auto const color = get_tag_color(tag.type);
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
		ImGui::TextColored(color, "%s", tag.value.data());
	}
}

auto TagEditor::update() -> Action {
	update_header();
	update_tags();
	ImGui::Separator();
	update_new_tag();
	ImGui::Separator();
	auto ret = update_buttons();
	if (ret) { ret = compute_replacement(); }
	return ret ? Action::ReplaceTags : Action::None;
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

	for (auto [index, tag] : std::views::enumerate(m_tags)) {
		ImGui::BeginDisabled(tag.is_input() && !tag.is_primary());
		m_dirty |= ImGui::Checkbox(klib::FixedString{"##keep{}", index}.c_str(), &tag.should_keep);
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (tag.is_input()) {
			ImGui::PushStyleColor(ImGuiCol_Text, get_tag_color(tag.in.type));
			ImGui::TextUnformatted(tag.in.value.data());
			ImGui::PopStyleColor();
		} else {
			ImGui::TextUnformatted(tag.new_value.data());
			if (!tag.should_keep) {
				m_tags.erase(m_tags.begin() + std::ptrdiff_t(index));
				break;
			}
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
		m_tags.push_back(Tag{.new_value = repoint_through(*m_tag_storage, new_tag)});
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
	if (std::ranges::all_of(m_tags, [](Tag const& tag) { return tag.is_input() && tag.should_keep; })) { return false; }

	for (auto& tag : m_tags) {
		if (!tag.is_input()) {
			m_replacement.push_back(tag.new_value);
			continue;
		}
		if (!tag.should_keep || !tag.is_primary()) { continue; }
		m_replacement.push_back(tag.in.value);
	}

	return true;
}
} // namespace xtag::gui::ui::widget

#pragma once
#include "ui/action.hpp"
#include "ui/widget/im_input_text.hpp"
#include "xtag/string_set.hpp"
#include "xtag/types.hpp"
#include <vector>

namespace xtag::gui::ui::widget {
class TagEditor {
  public:
	static void update_short_tags(std::span<ScanTag const> tags);

	void extract_tags(Entry const& selected);

	auto update() -> Action;

	[[nodiscard]] auto get_replacement() const -> std::span<std::string_view const> { return m_replacement; }

  private:
	struct Tag {
		[[nodiscard]] auto is_input() const -> bool { return new_value.empty(); }
		[[nodiscard]] auto is_primary() const -> bool { return in.type == TagType::Primary; }

		ScanTag in{};
		bool should_keep{true};
		std::string_view new_value{};
	};

	void update_header() const;
	void update_tags();
	void update_new_tag();
	[[nodiscard]] auto update_buttons() const -> bool;

	auto compute_replacement() -> bool;

	StringSet m_tag_set{};

	std::vector<Tag> m_tags{};
	ImInputText m_input{};
	std::vector<std::string_view> m_replacement{};

	bool m_dirty{};
};
} // namespace xtag::gui::ui::widget

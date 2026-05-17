#pragma once
#include "ui/widget/im_input_text.hpp"
#include "xtag/types.hpp"
#include <vector>

namespace xtag::gui::ui::widget {
class TagEditor {
  public:
	static constexpr auto label_v = klib::CString{"tag_editor"};

	static void update_short_tags(std::span<ScanTag const> tags);

	void set_should_open(Entry const& selected);

	auto update() -> bool;

	[[nodiscard]] auto get_replacement() const -> std::span<std::string const> { return m_replacement; }

  private:
	struct In {
		ScanTag tag{};
		bool should_keep{true};
	};

	struct List {
		void clear(std::size_t ins_reserve = 0);

		std::vector<In> ins{};
		std::vector<std::string> outs{};
	};

	void update_header() const;
	void update_tags();
	void update_new_tag();
	[[nodiscard]] auto update_buttons() const -> bool;

	void compute_replacement();

	List m_list{};
	ImInputText m_input{};
	std::vector<std::string> m_replacement{};

	bool m_should_open{};
	bool m_dirty{};
};

constexpr auto get_tag_color(TagType const type) {
	switch (type) {
	case TagType::Primary: return ImVec4{0.8f, 0.8f, 0.0f, 1.0f};
	case TagType::Inherited: return ImVec4{0.0f, 0.8f, 1.0f, 1.0f};
	default: return ImVec4{0.5f, 0.5f, 0.5f, 1.0f};
	}
}
} // namespace xtag::gui::ui::widget

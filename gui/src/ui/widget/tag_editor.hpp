#pragma once
#include "ui/widget/im_input_text.hpp"
#include "xtag/types.hpp"
#include <vector>

namespace xtag::gui::ui::widget {
class TagEditor {
  public:
	static void update_short_tags(std::span<ScanTag const> tags);

	void extract_tags(Entry const& selected);

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

	auto compute_replacement() -> bool;

	List m_list{};
	ImInputText m_input{};
	std::vector<std::string> m_replacement{};

	bool m_dirty{};
};
} // namespace xtag::gui::ui::widget

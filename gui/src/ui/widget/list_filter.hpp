#pragma once
#include "ui/widget/im_input_text.hpp"

namespace xtag::gui::ui::widget {
struct ListFilter {
	void update();

	ImInputText allow{};
	ImInputText block{};
};
} // namespace xtag::gui::ui::widget

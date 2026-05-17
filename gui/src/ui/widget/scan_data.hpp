#pragma once

namespace xtag::gui::ui::widget {
struct ScanData {
	void update();

	bool include_files{false};
	int depth{3};
};
} // namespace xtag::gui::ui::widget

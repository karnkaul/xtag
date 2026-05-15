#pragma once
#include <filesystem>

namespace xtag::gui::ui {
namespace fs = std::filesystem;

struct ScanData {
	fs::path root{};
	bool include_files{false};
	int depth{3};
};
} // namespace xtag::gui::ui

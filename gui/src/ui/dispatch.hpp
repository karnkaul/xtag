#pragma once
#include "ui/object.hpp"
#include <imgui.h>
#include <filesystem>

namespace xtag::gui::ui {
namespace fs = std::filesystem;

class IDispatch : public Object {
  public:
	virtual void shutdown() = 0;
	virtual void refresh_root_directory() = 0;
	virtual void replace_tags(fs::path const& path, std::span<std::string_view const> tags) = 0;

	virtual void open_test_modal() = 0;
};
} // namespace xtag::gui::ui

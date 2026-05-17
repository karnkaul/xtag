#pragma once
#include "klib/string/c_string.hpp"
#include <imgui.h>
#include <string_view>
#include <vector>

namespace xtag::gui::ui::widget {
class ImInputText {
  public:
	static constexpr std::size_t default_size_v{128};

	explicit ImInputText(std::size_t max_size = default_size_v) { set_max_size(max_size); }

	auto update(klib::CString label, int flags = 0) -> bool {
		if (m_buffer.empty()) { m_buffer.resize(default_size_v); }
		return ImGui::InputText(label.c_str(), m_buffer.data(), m_buffer.size(), flags);
	}

	[[nodiscard]] auto as_view() const -> std::string_view { return m_buffer.data(); }

	[[nodiscard]] auto get_max_size() const -> std::size_t { return m_buffer.size(); }
	void set_max_size(std::size_t const max_size) { m_buffer.resize(max_size, '\0'); }

  private:
	std::vector<char> m_buffer{};
};
} // namespace xtag::gui::ui::widget

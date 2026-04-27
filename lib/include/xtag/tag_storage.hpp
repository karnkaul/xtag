#pragma once
#include <string>
#include <unordered_set>

namespace xtag {
class TagStorage {
  public:
	auto insert_tag(std::string tag) -> std::string_view;

	[[nodiscard]] auto get_tags() const -> std::unordered_set<std::string> const& { return m_tags; }
	void clear_tags() { m_tags.clear(); }

  private:
	std::unordered_set<std::string> m_tags{};
};
} // namespace xtag

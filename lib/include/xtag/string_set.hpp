#pragma once
#include <string>
#include <unordered_set>

namespace xtag {
/// \brief Heterogeneous string hasher.
struct StringHash : std::hash<std::string_view> {
	using is_transparent = void;
};

/// \brief Centralized storage for string keys.
using StringSet = std::unordered_set<std::string, StringHash, std::equal_to<>>;

[[nodiscard]] auto repoint_through(StringSet& out, std::string_view key) -> std::string_view;
} // namespace xtag

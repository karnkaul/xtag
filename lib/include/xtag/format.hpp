#pragma once
#include "xtag/types.hpp"

namespace xtag {
[[nodiscard]] auto format_file(Entry const& file) -> std::string;
[[nodiscard]] auto format_tree(Entry const& directory) -> std::string;
} // namespace xtag

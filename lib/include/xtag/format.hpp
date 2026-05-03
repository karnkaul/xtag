#pragma once
#include "xtag/types.hpp"
#include <functional>

namespace xtag {
struct FormatParams {
	std::string_view path_header{"path"};
	std::string_view tags_header{"tags"};
	std::move_only_function<fs::path(fs::path const&)> transform_path{};
};

[[nodiscard]] auto format_table(std::span<TaggedEntry const> entries, FormatParams params = {}) -> std::string;
} // namespace xtag

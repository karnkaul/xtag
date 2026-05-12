#pragma once
#include "xtag/types.hpp"

namespace xtag::util {
inline constexpr std::string_view inherited_prefix_v{"*"};

void join_to(std::string& out, std::string_view item, std::string_view delimiter);
[[nodiscard]] auto join(std::string_view item, std::string_view delimiter);

[[nodiscard]] auto as_string(ScanTag const& scan_tag, std::string_view inherited_prefix = inherited_prefix_v) -> std::string;
[[nodiscard]] auto as_strings(std::span<ScanTag const> scan_tags, std::string_view inherited_prefix = inherited_prefix_v) -> std::vector<std::string>;

struct JoinParams {
	std::string_view inherited_prefix{inherited_prefix_v};
	std::string_view delimiter{", "};
	std::string_view overflow{"..."};
	TagType type_filter{TagType::Primary | TagType::Inherited};
	int max_count{0};
};

void join_tags_to(std::string& out, std::span<ScanTag const> tags, JoinParams const& params = {});
[[nodiscard]] auto join_tags(std::span<ScanTag const> tags, JoinParams const& params = {}) -> std::string;
} // namespace xtag::util

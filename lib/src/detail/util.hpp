#pragma once
#include "xtag/string_set.hpp"
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace xtag::detail {
void deserialize_tags_to(StringSet& out_set, std::vector<std::string_view>& out_tags, std::string_view serialized);
void serialize_tags_to(std::string& out, std::span<std::string_view const> tags);
} // namespace xtag::detail

#pragma once
#include "xtag/tag_storage.hpp"
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace xtag::detail {
void deserialize_tags_to(TagStorage& storage, std::vector<std::string_view>& out, std::string_view serialized);
void serialize_tags_to(std::string& out, std::span<std::string_view const> tags);
} // namespace xtag::detail

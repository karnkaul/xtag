#pragma once
#include "xtag/string_set.hpp"
#include <functional>
#include <span>

namespace xtag::detail {
void serialize_tags_to(std::string& out, std::span<std::string_view const> tags);

using OnTagDeserialized = std::move_only_function<void(std::string_view)>;
void deserialize_tags(StringSet& out_set, std::string_view serialized, OnTagDeserialized per_tag);

void join_to(std::string& out, std::string_view item, std::string_view delimiter);
} // namespace xtag::detail

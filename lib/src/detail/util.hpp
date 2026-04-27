#pragma once
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace xtag::detail {
void deserialize_tags_to(std::vector<std::string>& out, std::string_view serialized);
[[nodiscard]] auto deserialize_tags(std::string_view serialized) -> std::vector<std::string>;

void serialize_tags_to(std::string& out, std::span<std::string_view const> tags);
[[nodiscard]] auto serialize_tags(std::span<std::string_view const> tags) -> std::string;

void combine_tags_to(std::vector<std::string_view>& out, std::span<std::string_view const> a, std::span<std::string_view const> b);
[[nodiscard]] auto combine_tags(std::span<std::string_view const> a, std::span<std::string_view const> b) -> std::vector<std::string_view>;
} // namespace xtag::detail

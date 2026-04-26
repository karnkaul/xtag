#pragma once
#include "xtag/result.hpp"
#include <filesystem>
#include <span>
#include <vector>

namespace xtag {
namespace fs = std::filesystem;

auto get_tags_to(std::vector<std::string>& out, fs::path const& path) -> Result<void>;
[[nodiscard]] auto get_tags(fs::path const& path) -> Result<std::vector<std::string>>;

auto replace_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;
auto append_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;

auto erase_tags(fs::path const& path) -> Result<void>;
} // namespace xtag

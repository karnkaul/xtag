#pragma once
#include "xtag/result.hpp"
#include "xtag/tag_storage.hpp"
#include <vector>

namespace xtag {
class Instance {
  public:
	struct Storage {
		TagStorage tags{};
	};

	[[nodiscard]] auto get_tags(fs::path const& path) -> Result<TaggedEntry>;

	auto replace_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;
	auto append_tags(fs::path const& path, std::span<std::string_view const> tags) -> Result<void>;

	static auto erase_tags(fs::path const& path) -> Result<void>;

	[[nodiscard]] auto scan_tagged(fs::path const& directory, ScanParams const& params = {}) -> std::vector<TaggedEntry>;

	Storage storage{};

  private:
	[[nodiscard]] auto wipe_buffer() -> std::string&;

	std::string m_buffer{};
};
} // namespace xtag

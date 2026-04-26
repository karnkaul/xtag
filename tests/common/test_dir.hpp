#pragma once
#include <filesystem>

namespace fs = std::filesystem;

namespace xtag {
class TestDir {
  public:
	TestDir(TestDir const&) = delete;
	TestDir(TestDir&&) = delete;
	TestDir& operator=(TestDir const&) = delete;
	TestDir& operator=(TestDir&&) = delete;

	explicit TestDir(fs::path path = "_runtime");
	~TestDir();

	[[nodiscard]] auto get_path() const -> fs::path const& { return m_path; }

	// NOLINTNEXTLINE(modernize-use-nodiscard)
	auto create_empty_file(fs::path const& subpath) const -> fs::path;

  private:
	fs::path m_path{};
};
} // namespace xtag

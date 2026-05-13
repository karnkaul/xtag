#include "common/fixture.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace {
using namespace std::string_view_literals;

TEST_CASE(scan_directory) {
	auto fixture = xtag::Fixture{};

	auto const dir_a = fixture.test_dir.create_directory("dir_a");
	auto const dir_b = fixture.test_dir.create_directory("dir_a/dir_b");
	auto const file_c = fixture.test_dir.create_empty_file("dir_a/file_c");
	auto const file_d = fixture.test_dir.create_empty_file("dir_a/dir_b/file_d");

	auto const tags_a = std::vector<std::string_view>{"foo", "bar"};
	[[maybe_unused]] auto res = fixture.instance.replace_tags(dir_a, tags_a);

	auto tags_b = std::vector<std::string_view>{"baz"};
	res = fixture.instance.replace_tags(dir_b, tags_b);

	auto const tags_c = std::vector<std::string_view>{"fubar"};
	res = fixture.instance.replace_tags(file_c, tags_c);

	static auto const expect_tags = [](std::span<xtag::ScanTag const> scan_tags, std::span<std::string_view const> primary,
									   std::span<std::string_view const> inherited) {
		auto const contains_tag = [&](std::string_view const scan_tag, xtag::TagType const type) {
			return std::ranges::find(scan_tags, xtag::ScanTag{.value = scan_tag, .type = type}) != scan_tags.end();
		};
		return std::ranges::all_of(primary, [&](std::string_view const tag) { return contains_tag(tag, xtag::TagType::Primary); }) &&
			   std::ranges::all_of(inherited, [&](std::string_view const tag) { return contains_tag(tag, xtag::TagType::Inherited); });
	};

	auto info = xtag::ScanInfo{.depth = 10};
	info.filter.tag_type |= xtag::TagType::Untagged;

	auto result = fixture.instance.scan_directory(fixture.test_dir.get_path(), info);
	ASSERT(result);
	EXPECT(result->path == fs::absolute(fixture.test_dir.get_path()));

	result->sort_recursive();

	ASSERT(result->subentries.size() == 1);
	auto const& res_dir_a = result->subentries[0];
	EXPECT(res_dir_a.type == xtag::EntryType::Directory);
	EXPECT(expect_tags(res_dir_a.tags, tags_a, {}));
	ASSERT(res_dir_a.subentries.size() == 2);

	auto const& res_dir_b = res_dir_a.subentries[0];
	EXPECT(res_dir_b.type == xtag::EntryType::Directory);
	EXPECT(res_dir_b.path == fs::absolute(dir_b));
	EXPECT(expect_tags(res_dir_b.tags, tags_b, tags_a));

	auto const& res_file_c = res_dir_a.subentries[1];
	EXPECT(res_file_c.type == xtag::EntryType::File);
	EXPECT(res_file_c.path == fs::absolute(file_c));
	EXPECT(expect_tags(res_file_c.tags, tags_c, tags_a));

	ASSERT(res_dir_b.subentries.size() == 1);
	auto const& res_file_d = res_dir_b.subentries[0];
	EXPECT(res_file_d.type == xtag::EntryType::File);
	EXPECT(res_file_d.path == fs::absolute(file_d));
	EXPECT(expect_tags(res_file_d.tags, {}, tags_b));
}
} // namespace

#include "common/fixture.hpp"
#include "klib/unit_test/unit_test.hpp"
#include <algorithm>

namespace {
using namespace std::string_view_literals;

TEST_CASE(scan_tagged) {
	auto fixture = xtag::Fixture{};

	auto const dir_a = fixture.test_dir.create_directory("dir_a");
	auto const dir_b = fixture.test_dir.create_directory("dir_a/dir_b");
	auto const file_c = fixture.test_dir.create_empty_file("dir_a/file_c");
	auto const file_d = fixture.test_dir.create_empty_file("dir_a/dir_b/file_d");

	auto const tags_a = std::vector<std::string_view>{"foo", "bar"};
	[[maybe_unused]] auto res = fixture.instance.replace_tags(dir_a, tags_a);

	auto tags_b = tags_a;
	tags_b.emplace_back("baz");
	res = fixture.instance.replace_tags(dir_b, tags_b);

	auto const tags_c = std::vector<std::string_view>{"fubar"};
	res = fixture.instance.replace_tags(file_c, tags_c);

	static auto const sort_pred = [](xtag::Entry const& a, xtag::Entry const& b) { return a.path < b.path; };
	static auto const expect_tags = [](xtag::Entry const& e, std::span<std::string_view const> tags, xtag::TagType const type) {
		auto const pred = [&](std::string_view const scan_tag) {
			return std::ranges::find(e.scan_tags, xtag::ScanTag{.value = scan_tag, .type = type}) != e.scan_tags.end();
		};
		return std::ranges::all_of(tags, pred);
	};

	auto info = xtag::ScanInfo{.depth = 10};
	auto tagged = fixture.instance.scan_tagged(fixture.test_dir.get_path(), info);
	ASSERT(tagged.size() == 4);
	std::ranges::sort(tagged, sort_pred);
	EXPECT(expect_tags(tagged[0], tags_a, xtag::TagType::Primary));
	// TODO: inherited

	static constexpr auto tag_filter_v = std::array{
		"baz"sv,
	};
	info.filter.tags = tag_filter_v;
	tagged = fixture.instance.scan_tagged(fixture.test_dir.get_path(), info);
	ASSERT(tagged.size() == 2);
	std::ranges::sort(tagged, sort_pred);
	EXPECT(expect_tags(tagged[0], tags_a, xtag::TagType::Primary));
	// TODO: inherited
}
} // namespace

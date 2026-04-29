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
	fixture.instance.replace_tags(dir_a, tags_a);

	auto tags_b = tags_a;
	tags_b.emplace_back("baz");
	fixture.instance.replace_tags(dir_b, tags_b);

	auto const tags_c = std::vector<std::string_view>{"fubar"};
	fixture.instance.replace_tags(file_c, tags_c);

	static constexpr auto sort_pred = [](xtag::TaggedEntry const& a, xtag::TaggedEntry const& b) { return a.path < b.path; };

	auto params = xtag::ScanParams{.depth = 10};
	auto tagged = fixture.instance.scan_tagged(fixture.test_dir.get_path(), params);
	ASSERT(tagged.size() == 3);
	std::ranges::sort(tagged, sort_pred);
	EXPECT(tagged[0].tags == tags_a);
	EXPECT(tagged[1].tags == tags_b);
	EXPECT(tagged[2].tags == tags_c);

	static constexpr auto tag_filter_v = std::array{
		"bar"sv,
	};
	params.tag_filter = tag_filter_v;
	tagged = fixture.instance.scan_tagged(fixture.test_dir.get_path(), params);
	ASSERT(tagged.size() == 2);
	std::ranges::sort(tagged, sort_pred);
	EXPECT(tagged[0].tags == tags_a);
	EXPECT(tagged[1].tags == tags_b);
}
} // namespace

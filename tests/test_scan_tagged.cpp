#include "common/fixture.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace {
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

	auto const params = xtag::ScanParams{.depth = 10};
	auto const tagged = fixture.instance.scan_tagged(fixture.test_dir.get_path(), params);
	ASSERT(tagged.size() == 3);
	for (auto const& entry : tagged) {
		if (entry.path == dir_a) {
			EXPECT(entry.tags == tags_a);
		} else if (entry.path == dir_b) {
			EXPECT(entry.tags == tags_b);
		} else if (entry.path == file_c) {
			EXPECT(entry.tags == tags_c);
		} else {
			EXPECT(false);
		}
	}
}
} // namespace

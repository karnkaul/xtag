#include "detail/util.hpp"
#include "klib/unit_test/unit_test.hpp"
#include <array>
#include <string_view>

namespace {
TEST_CASE(tag_serialize) {
	auto const tags = std::array{
		"foo",
		"bar",
		"baz",
		"with space",
	};
	auto serialized = std::string{};
	xtag::detail::serialize_tags_to(serialized, tags);
	EXPECT(serialized == "foo|bar|baz|with space");
}

TEST_CASE(tag_deserialize) {
	static constexpr auto serialized_v = std::string_view{"foo|bar|baz|with space"};
	auto tags = std::vector<std::string>{};
	xtag::detail::deserialize_tags_to(tags, serialized_v);
	ASSERT(tags.size() == 4);
	EXPECT(tags[0] == "foo");
	EXPECT(tags[1] == "bar");
	EXPECT(tags[2] == "baz");
	EXPECT(tags[3] == "with space");
}
} // namespace

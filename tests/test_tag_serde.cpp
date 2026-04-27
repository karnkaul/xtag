#include "detail/util.hpp"
#include "klib/unit_test/unit_test.hpp"
#include <array>
#include <string_view>

namespace {
using namespace std::string_view_literals;

TEST_CASE(tag_serialize) {
	auto const tags = std::array{
		"foo"sv,
		"bar"sv,
		"baz"sv,
		"with space"sv,
	};
	auto const serialized = xtag::detail::serialize_tags(tags);
	EXPECT(serialized == "foo|bar|baz|with space");
}

TEST_CASE(tag_deserialize) {
	static constexpr auto serialized_v = std::string_view{"foo|bar|baz|with space"};
	auto const tags = xtag::detail::deserialize_tags(serialized_v);
	ASSERT(tags.size() == 4);
	EXPECT(tags[0] == "foo");
	EXPECT(tags[1] == "bar");
	EXPECT(tags[2] == "baz");
	EXPECT(tags[3] == "with space");
}
} // namespace

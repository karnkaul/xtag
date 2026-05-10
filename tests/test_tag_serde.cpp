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
	auto serialized = std::string{};
	xtag::detail::serialize_tags_to(serialized, tags);
	EXPECT(serialized == "foo,bar,baz,with space");
}

TEST_CASE(tag_deserialize) {
	static constexpr auto serialized_v = std::string_view{"foo,bar,baz,with space"};
	auto tags = std::vector<std::string_view>{};
	auto storage = xtag::StringSet{};
	auto const per_tag = [&](std::string_view const tag) { tags.push_back(tag); };
	xtag::detail::deserialize_tags(storage, serialized_v, per_tag);
	ASSERT(tags.size() == 4);
	EXPECT(tags[0] == "foo");
	EXPECT(tags[1] == "bar");
	EXPECT(tags[2] == "baz");
	EXPECT(tags[3] == "with space");
}
} // namespace

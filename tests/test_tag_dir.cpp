#include "common/fixture.hpp"
#include "klib/unit_test/unit_test.hpp"
#include "xtag/types.hpp"
#include "xtag/xtag.hpp"
#include <string_view>

namespace {
using ErrorType = xtag::Error::Type;

TEST_CASE(write_tags) {
	auto tag_storage = xtag::TagStorage{};
	auto tags = xtag::get_tags(tag_storage, "nonexistent_path");
	ASSERT(!tags);
	EXPECT(tags.error().type == ErrorType::InvalidArgument);

	auto fixture = xtag::Fixture{};
	tags = xtag::get_tags(fixture.tag_storage, fixture.test_dir.get_path());
	ASSERT(tags);
	EXPECT(tags->empty());

	auto tags_to_write = std::vector<std::string_view>{"foo", "bar"};
	std::ranges::sort(tags_to_write);

	auto result = xtag::replace_tags(fixture.test_dir.get_path(), tags_to_write);
	EXPECT(result);

	tags = xtag::get_tags(fixture.tag_storage, fixture.test_dir.get_path());
	ASSERT(tags);
	ASSERT(tags->size() == 2);
	EXPECT(tags->at(0) == tags_to_write[0]);
	EXPECT(tags->at(1) == tags_to_write[1]);
}

TEST_CASE(replace_tags) {
	auto fixture = xtag::Fixture{};

	auto tags_to_write = std::vector<std::string_view>{"foo", "bar"};
	auto result = xtag::replace_tags(fixture.test_dir.get_path(), tags_to_write);
	EXPECT(result);

	tags_to_write = std::vector<std::string_view>{"baz", "fubar"};
	result = xtag::replace_tags(fixture.test_dir.get_path(), tags_to_write);
	EXPECT(result);

	auto tags = xtag::get_tags(fixture.tag_storage, fixture.test_dir.get_path());
	ASSERT(tags);
	ASSERT(tags->size() == 2);
	EXPECT(tags->at(0) == tags_to_write[0]);
	EXPECT(tags->at(1) == tags_to_write[1]);
}

TEST_CASE(append_tags) {
	auto fixture = xtag::Fixture{};

	auto tags_to_write = std::vector<std::string_view>{"foo", "bar"};
	auto result = xtag::replace_tags(fixture.test_dir.get_path(), tags_to_write);
	EXPECT(result);

	auto tags_to_append = std::vector<std::string_view>{"baz", "fubar"};
	result = xtag::append_tags(fixture.tag_storage, fixture.test_dir.get_path(), tags_to_append);
	EXPECT(result);

	auto tags = xtag::get_tags(fixture.tag_storage, fixture.test_dir.get_path());
	ASSERT(tags);
	ASSERT(tags->size() == 4);
	EXPECT(tags->at(0) == tags_to_write[0]);
	EXPECT(tags->at(1) == tags_to_write[1]);
	EXPECT(tags->at(2) == tags_to_append[0]);
	EXPECT(tags->at(3) == tags_to_append[1]);
}

TEST_CASE(erase_tags) {
	auto fixture = xtag::Fixture{};

	auto tags_to_write = std::vector<std::string_view>{"foo", "bar"};
	auto result = xtag::replace_tags(fixture.test_dir.get_path(), tags_to_write);
	EXPECT(result);

	result = xtag::erase_tags(fixture.test_dir.get_path());
	EXPECT(result);

	auto tags = xtag::get_tags(fixture.tag_storage, fixture.test_dir.get_path());
	ASSERT(tags);
	EXPECT(tags->empty());
}
} // namespace

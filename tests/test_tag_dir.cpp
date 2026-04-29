#include "common/fixture.hpp"
#include "klib/unit_test/unit_test.hpp"
#include "xtag/instance.hpp"
#include "xtag/types.hpp"
#include <string_view>

namespace {
using ErrorType = xtag::Error::Type;

TEST_CASE(get_tags_nonexistent_path) {
	auto instance = xtag::Instance{};
	auto tags = instance.get_tags("nonexistent_path");
	ASSERT(!tags);
	EXPECT(tags.error().type == ErrorType::InvalidArgument);
}

TEST_CASE(write_tags) {
	auto fixture = xtag::Fixture{};
	auto entry = fixture.instance.get_tags(fixture.test_dir.get_path());
	ASSERT(entry);
	EXPECT(entry->tags.empty());

	auto tags_to_write = std::vector<std::string_view>{"foo", "bar"};

	auto result = fixture.instance.replace_tags(fixture.test_dir.get_path(), tags_to_write);
	EXPECT(result);

	entry = fixture.instance.get_tags(fixture.test_dir.get_path());
	ASSERT(entry);
	ASSERT(entry->tags.size() == 2);
	EXPECT(entry->tags.at(0) == tags_to_write[0]);
	EXPECT(entry->tags.at(1) == tags_to_write[1]);
}

TEST_CASE(replace_tags) {
	auto fixture = xtag::Fixture{};

	auto tags_to_write = std::vector<std::string_view>{"foo", "bar"};
	auto result = fixture.instance.replace_tags(fixture.test_dir.get_path(), tags_to_write);
	EXPECT(result);

	tags_to_write = std::vector<std::string_view>{"baz", "fubar"};
	result = fixture.instance.replace_tags(fixture.test_dir.get_path(), tags_to_write);
	EXPECT(result);

	auto entry = fixture.instance.get_tags(fixture.test_dir.get_path());
	ASSERT(entry);
	ASSERT(entry->tags.size() == 2);
	EXPECT(entry->tags.at(0) == tags_to_write[0]);
	EXPECT(entry->tags.at(1) == tags_to_write[1]);
}

TEST_CASE(append_tags) {
	auto fixture = xtag::Fixture{};

	auto tags_to_write = std::vector<std::string_view>{"foo", "bar"};
	auto result = fixture.instance.replace_tags(fixture.test_dir.get_path(), tags_to_write);
	EXPECT(result);

	auto tags_to_append = std::vector<std::string_view>{"baz", "fubar"};
	result = fixture.instance.append_tags(fixture.test_dir.get_path(), tags_to_append);
	EXPECT(result);

	auto entry = fixture.instance.get_tags(fixture.test_dir.get_path());
	ASSERT(entry);
	ASSERT(entry->tags.size() == 4);
	EXPECT(entry->tags.at(0) == tags_to_write[0]);
	EXPECT(entry->tags.at(1) == tags_to_write[1]);
	EXPECT(entry->tags.at(2) == tags_to_append[0]);
	EXPECT(entry->tags.at(3) == tags_to_append[1]);
}

TEST_CASE(erase_tags) {
	auto fixture = xtag::Fixture{};

	auto tags_to_write = std::vector<std::string_view>{"foo", "bar"};
	auto result = fixture.instance.replace_tags(fixture.test_dir.get_path(), tags_to_write);
	EXPECT(result);

	result = xtag::Instance::erase_tags(fixture.test_dir.get_path());
	EXPECT(result);

	auto entry = fixture.instance.get_tags(fixture.test_dir.get_path());
	ASSERT(entry);
	EXPECT(entry->tags.empty());
}
} // namespace

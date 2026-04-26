#include "common/fixture.hpp"
#include "klib/string/c_string.hpp"
#include "klib/unit_test/unit_test.hpp"
#include "xtag/types.hpp"
#include "xtag/xattr.hpp"
#include <cerrno>

namespace {
TEST_CASE(xattr_api) {
	auto const fixture = xtag::Fixture{};

	static constexpr auto name_v = klib::CString{"user.foo"};
	auto value = xtag::xattr::get("invalid_path", name_v);
	ASSERT(!value);
	EXPECT(value.error().type == xtag::Error::Type::InvalidArgument);
	EXPECT(errno == 0);

	auto const path = (fixture.test_dir.get_path() / "a").generic_string();
	value = xtag::xattr::get(path, name_v);
	ASSERT(!value);
	EXPECT(value.error().type == xtag::Error::Type::InvalidArgument);
	EXPECT(errno == 0);

	fixture.test_dir.create_empty_file("a");

	value = xtag::xattr::get(path, name_v);
	ASSERT(!value);
	EXPECT(value.error().type == xtag::Error::Type::NoData);
	EXPECT(errno == 0);

	auto result = xtag::xattr::set(path, name_v, "bar");
	EXPECT(result);

	value = xtag::xattr::get(path, name_v);
	ASSERT(value);
	EXPECT(*value == "bar");

	result = xtag::xattr::remove(path, name_v);
	EXPECT(result);

	value = xtag::xattr::get(path, name_v);
	ASSERT(!value);
	EXPECT(value.error().type == xtag::Error::Type::NoData);
	EXPECT(errno == 0);
}
} // namespace

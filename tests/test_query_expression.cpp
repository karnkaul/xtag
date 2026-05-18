#include "klib/unit_test/unit_test.hpp"
#include "xtag/query.hpp"
#include <array>

namespace {
TEST_CASE(query_parse) {
	using Scope = xtag::query::Scope;
	static constexpr std::string_view text_v = R"(foo -bar t=some-tag -f="quoted string")";

	auto const expression = xtag::query::parse(text_v);
	ASSERT(expression.predicates.size() == 4);

	auto const& foo = expression.predicates[0];
	EXPECT(foo.pattern == "foo");
	EXPECT(foo.scope == (Scope::Filename | Scope::Tag));
	EXPECT(!foo.invert);

	auto const& bar = expression.predicates[1];
	EXPECT(bar.pattern == "bar");
	EXPECT(bar.scope == (Scope::Filename | Scope::Tag));
	EXPECT(bar.invert);

	auto const& some_tag = expression.predicates[2];
	EXPECT(some_tag.pattern == "some-tag");
	EXPECT(some_tag.scope == Scope::Tag);
	EXPECT(!some_tag.invert);

	auto const& quoted_string = expression.predicates[3];
	EXPECT(quoted_string.pattern == "quoted string");
	EXPECT(quoted_string.scope == Scope::Filename);
	EXPECT(quoted_string.invert);
}

TEST_CASE(query_expression) {
	static constexpr auto scan_tags_v = std::array{
		xtag::ScanTag{.value = "fixer"},
		xtag::ScanTag{.value = "vertical"},
		xtag::ScanTag{.value = "knob"},
	};
	static constexpr auto filename_v = std::string_view{"foo/bar.txt"};

	auto expression = xtag::query::parse("tags=er");
	EXPECT(expression.is_match(filename_v, scan_tags_v));

	expression = xtag::query::parse("tags=knob");
	EXPECT(expression.is_match(filename_v, scan_tags_v));

	expression = xtag::query::parse("tags=none");
	EXPECT(!expression.is_match(filename_v, scan_tags_v));

	expression = xtag::query::parse("tags=er tags=knob");
	EXPECT(expression.is_match(filename_v, scan_tags_v));

	expression = xtag::query::parse("filename=foo");
	EXPECT(expression.is_match(filename_v, scan_tags_v));

	expression = xtag::query::parse("tags=er -f=bar");
	EXPECT(!expression.is_match(filename_v, scan_tags_v));
}
} // namespace

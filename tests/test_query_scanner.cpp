#include "detail/query_scanner.hpp"

namespace {
class Fixture {
  public:
	constexpr auto operator()() {
		if (!scan_next()) { return false; }
		if (!expect_token(Type::String, "foo")) { return false; }

		if (!scan_next()) { return false; }
		if (!expect_token(Type::String, "t")) { return false; }
		if (!scan_next()) { return false; }
		if (!expect_token(Type::Equals, "=")) { return false; }
		if (!scan_next()) { return false; }
		if (!expect_token(Type::String, "some-tag")) { return false; }

		if (!scan_next()) { return false; }
		if (!expect_token(Type::String, "-f")) { return false; }
		if (!scan_next()) { return false; }
		if (!expect_token(Type::Equals, "=")) { return false; }
		if (!scan_next()) { return false; }

		return expect_token(Type::String, "quoted string");
	}

  private:
	static constexpr std::string_view text_v = R"(foo t=some-tag -f="quoted string")";
	using Type = xtag::query::TokenType;

	constexpr auto scan_next() -> bool { return m_scanner.next(m_token); }

	[[nodiscard]] constexpr auto expect_token(Type const type, std::string_view const lexeme) const -> bool {
		return m_token.type == type && m_token.lexeme == lexeme;
	}

	xtag::query::Scanner m_scanner{text_v};
	xtag::query::Token m_token{};
};

static_assert(Fixture{}());
} // namespace

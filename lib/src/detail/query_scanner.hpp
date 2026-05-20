#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace xtag::query {
enum class TokenType : std::int8_t { String, Equals };

struct Token {
	using Type = TokenType;

	Type type{};
	std::string_view lexeme{};
	std::size_t start_index{};
};

class Scanner {
  public:
	explicit constexpr Scanner(std::string_view const text) : m_text(text) {}

	constexpr auto next(Token& out) -> bool {
		trim_whitespace();
		if (at_end()) { return false; }
		out = scan_token();
		return true;
	}

  private:
	[[nodiscard]] static constexpr auto is_space(char const c) -> bool { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
	[[nodiscard]] static constexpr auto is_end_of_string(char const c) -> bool { return c == '=' || is_space(c); }

	[[nodiscard]] constexpr auto at_end() const -> bool { return m_cursor >= m_text.size(); }
	[[nodiscard]] constexpr auto current() const -> char { return at_end() ? '\0' : m_text.at(m_cursor); }

	constexpr void trim_whitespace() {
		while (!at_end() && is_space(m_text.at(m_cursor))) { ++m_cursor; }
	}

	[[nodiscard]] constexpr auto to_token(TokenType const type, std::size_t const length) -> Token {
		auto ret = Token{.type = type, .start_index = m_cursor};
		ret.lexeme = m_text.substr(m_cursor, length);
		m_text.remove_prefix(length);
		return ret;
	}

	[[nodiscard]] constexpr auto scan_token() -> Token {
		switch (current()) {
		case '=': return to_token(TokenType::Equals, 1);
		case '\"': return scan_quoted();
		default: return scan_string();
		}
	}

	[[nodiscard]] constexpr auto scan_quoted() -> Token {
		++m_cursor; // "
		auto const i = m_text.find('\"', m_cursor);
		if (i == std::string_view::npos) { return to_token(TokenType::String, m_text.size() - m_cursor); }
		auto const ret = to_token(TokenType::String, i - m_cursor);
		++m_cursor; // "
		return ret;
	}

	[[nodiscard]] constexpr auto scan_string() -> Token {
		auto length = 1uz;
		while (m_cursor + length < m_text.size() && !is_end_of_string(m_text.at(m_cursor + length))) { ++length; }
		return to_token(TokenType::String, length);
	}

	std::string_view m_text{};
	std::size_t m_cursor{};
};
} // namespace xtag::query

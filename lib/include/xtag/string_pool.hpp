#pragma once
#include <deque>
#include <string>

namespace xtag {
class StringPool {
  public:
	class Scope;

	void clear();

  private:
	std::deque<std::string> m_buffer{};
	std::size_t m_index{};
	bool m_bound_to_scope{};
};

class StringPool::Scope {
  public:
	Scope(Scope const&) = delete;
	Scope(Scope&&) = delete;
	Scope& operator=(Scope const&) = delete;
	Scope& operator=(Scope&&) = delete;

	explicit Scope(StringPool& pool);
	~Scope();

	[[nodiscard]] auto acquire() const -> std::string&;

  private:
	void release_all();

	StringPool& m_pool;
	bool m_active{};
};
} // namespace xtag

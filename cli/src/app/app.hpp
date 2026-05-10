#pragma once
#include "clap/result.hpp"
#include "command/command.hpp"
#include <memory>

namespace xtag::cli {
class App {
  public:
	[[nodiscard]] auto run(int argc, char const* const* argv) -> int;

  private:
	[[nodiscard]] auto parse_args(int argc, char const* const* argv) -> clap::Result;

	template <std::derived_from<Command> T, typename... Args>
		requires(std::constructible_from<T, Args...>)
	void add_command(Args&&... args) {
		m_commands.push_back(std::make_unique<T>(std::forward<Args>(args)...));
	}

	std::vector<std::unique_ptr<Command>> m_commands{};
	Instance m_instance{};
};
} // namespace xtag::cli

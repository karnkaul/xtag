#pragma once
#include "command/command.hpp"

namespace xtag::cli::command {
class List : public Command {
	static constexpr auto name_v = std::string_view{"list"};

	[[nodiscard]] auto get_name() const -> std::string_view final { return name_v; }
	auto get_parameters() -> std::vector<clap::Parameter> final;
	auto execute(Instance& instance) -> ExitCode final;

	std::string_view m_path{"."};
};
} // namespace xtag::cli::command

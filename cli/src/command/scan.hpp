#pragma once
#include "command/command.hpp"

namespace xtag::cli::command {
class Scan : public Command {
	static constexpr auto name_v = std::string_view{"scan"};

	[[nodiscard]] auto get_name() const -> std::string_view final { return name_v; }
	auto get_parameters() -> std::vector<clap::Parameter> final;
	auto execute(Instance& instance) -> ExitCode final;

	std::string_view m_path{"."};
	std::vector<std::string_view> m_tags{};
	std::string_view m_type{};
	int m_depth{10};
};
} // namespace xtag::cli::command

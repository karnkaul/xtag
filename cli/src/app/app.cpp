#include "app/app.hpp"
#include "clap/parameter.hpp"
#include "clap/parser.hpp"
#include "clap/spec.hpp"
#include "command/command.hpp"
#include "klib/string/c_string.hpp"
#include "log.hpp"
#include "xtag/build_version.hpp"
#include "xtag/panic.hpp"
#include <cstdlib>

namespace xtag::cli {
auto App::run(int argc, char const* const* argv) -> int {
	auto const parse_result = parse_args(argc, argv);
	if (parse_result.should_early_exit()) { return parse_result.return_code(); }

	log.debug("{}", build_version_v);

	auto const identifier = parse_result.command_identifier();
	auto const it = std::ranges::find_if(m_commands, [identifier](auto const& c) { return c->get_name() == identifier; });
	if (it == m_commands.end()) {
		log.error("unrecognized command: {}", identifier);
		return EXIT_FAILURE;
	}

	auto& command = **it;

	return int(command.execute());
}

auto App::parse_args(int argc, char const* const* argv) -> clap::Result {
	auto spec = clap::spec::Commands{
		.options =
			{

			},
		.program =
			{
				.version = build_version_str,
				.description = "video file formatter",
			},
	};
	spec.commands.reserve(m_commands.size());
	for (auto const& command : m_commands) { spec.commands.push_back(clap::command(command->get_name(), command->get_parameters(), command->get_help())); }

	auto parser = clap::Parser{std::move(spec)};
	return parser.parse_main(argc, argv);
}
} // namespace xtag::cli

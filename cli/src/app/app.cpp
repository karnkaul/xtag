#include "app/app.hpp"
#include "clap/parser.hpp"
#include "clap/spec.hpp"
#include "command/command.hpp"
#include "command/erase.hpp"
#include "command/list.hpp"
#include "command/scan.hpp"
#include "command/set.hpp"
#include "log.hpp"
#include "xtag/build_version.hpp"
#include <cstdlib>

namespace xtag::cli {
auto App::run(int argc, char const* const* argv) -> int {
	add_command<command::List>();
	add_command<command::Replace>();
	add_command<command::Append>();
	add_command<command::Erase>();
	add_command<command::Scan>();

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

	return int(command.execute(m_instance));
}

auto App::parse_args(int argc, char const* const* argv) -> clap::Result {
	auto spec = clap::spec::Commands{
		.options =
			{
				clap::named_option(m_instance.custom_attribute_name, "a,attr-name", "custom attribute name"),
			},
		.program =
			{
				.version = build_version_str,
				.description = "xattr tags manipulator",
			},
	};
	spec.commands.reserve(m_commands.size());
	for (auto const& command : m_commands) { spec.commands.push_back(clap::command(command->get_name(), command->get_parameters(), command->get_help())); }

	auto parser = clap::Parser{std::move(spec)};
	return parser.parse_main(argc, argv);
}
} // namespace xtag::cli

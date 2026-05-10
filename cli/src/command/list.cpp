#include "command/list.hpp"
#include "xtag/format.hpp"

namespace xtag::cli::command {
auto List::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::positional_optional(m_path, "PATH", "path (default=.)"),
	};
}

auto List::execute(Instance& instance) -> ExitCode {
	auto const result = instance.get_tags(m_path);
	if (!result) { return handle_error(result.error()); }

	auto const table = format_table({&*result, 1});
	std::println("{}", table);
	return ExitCode::Success;
}
} // namespace xtag::cli::command

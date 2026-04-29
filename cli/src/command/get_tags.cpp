#include "command/get_tags.hpp"

namespace xtag::cli::command {
auto GetTags::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::positional_optional(m_path, "PATH", "path (default=.)"),
	};
}

auto GetTags::execute(Instance& instance) -> ExitCode {
	auto const result = instance.get_tags(m_path);
	if (!result) { return handle_error(result.error()); }

	auto const table = format_table({&*result, 1});
	std::println("{}", table);
	return ExitCode::Success;
}
} // namespace xtag::cli::command

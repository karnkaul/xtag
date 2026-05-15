#include "command/list.hpp"
#include "xtag/formatter.hpp"

namespace xtag::cli::command {
auto List::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::positional_optional(m_path, "PATH", "path (default=.)"),
	};
}

auto List::execute(Instance& instance) -> ExitCode {
	auto const result = instance.get_tags(m_path);
	if (!result) { return handle_error(result.error()); }

	std::println("{}", Formatter{}.join(result->tags));

	return ExitCode::Success;
}
} // namespace xtag::cli::command

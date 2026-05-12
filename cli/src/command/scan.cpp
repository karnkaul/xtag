#include "command/scan.hpp"
#include "xtag/format.hpp"

namespace xtag::cli::command {
auto Scan::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::named_flag(m_include_files, "f,include-files"),
		clap::named_option(m_depth, "d,depth", "iteration depth (default: 10)"),
		clap::positional_required(m_path, "PATH"),
		clap::positional_list(m_tags, "TAGS"),
	};
}

auto Scan::execute(Instance& instance) -> ExitCode {
	auto info = ScanInfo{
		.filter = ScanFilter{.tags = m_tags, .include_files = m_include_files},
		.depth = m_depth,
	};

	auto const root = fs::absolute(m_path);
	auto result = instance.scan_directory(root, info);
	if (!result) { return handle_error(result.error()); }

	result->sort_recursive();
	auto const tree = format_tree(*result);
	std::println("{}", tree);
	return ExitCode::Success;
}
} // namespace xtag::cli::command

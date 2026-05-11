#include "app/app.hpp"
#include "clap/parser.hpp"
#include "clap/spec.hpp"
#include "log.hpp"
#include "xtag/build_version.hpp"
#include <cstdlib>

namespace xtag::gui {
auto App::run(int argc, char const* const* argv) -> int {
	auto const parse_result = parse_args(argc, argv);
	if (parse_result.should_early_exit()) { return parse_result.return_code(); }

	log.debug("{}", build_version_v);

	initialize();
	run_event_loop();
	return EXIT_SUCCESS;
}

auto App::parse_args(int argc, char const* const* argv) -> clap::Result {
	auto spec = clap::spec::Parameters{
		.parameters =
			{
				clap::named_option(m_instance.custom_attribute_name, "a,attr-name", "custom attribute name"),
			},
		.program =
			{
				.version = build_version_str,
				.description = "xattr tags manipulator",
			},
	};

	auto parser = clap::Parser{std::move(spec)};
	return parser.parse_main(argc, argv);
}

void App::update() { m_controller.update(); }

void App::initialize() {
	m_services.attach(&m_signals);
	m_services.attach(&m_instance);

	m_signals.shutdown.attach_to(m_slot, [this] { set_should_close_window(true); });

	m_controller.initialize(m_services);
}
} // namespace xtag::gui

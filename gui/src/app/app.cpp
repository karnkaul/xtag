#include "app/app.hpp"
#include "clap/parser.hpp"
#include "clap/spec.hpp"
#include "log.hpp"
#include "xtag/build_version.hpp"
#include "xtag/instance.hpp"
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

void App::stage_create() {
	gvdi::App::stage_create();

	ui::Controller::set_styles(ImGui::GetStyle());

	m_delta_time.restart();
}

auto App::create_glfw_window() -> GLFWwindow* {
	auto const title = std::format("xtag {}", build_version_str);
	return gvdi::App::create_windowed_window(title.c_str());
}

void App::on_path_drop(std::span<char const* const> paths) {
	if (paths.empty()) { return; }
	m_controller->on_drop(paths.front());
}

void App::update() {
	m_delta_time.update();
	m_controller->update();
	if (m_controller->get_state() == ui::Controller::State::Finished) { set_should_close_window(true); }
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

void App::initialize() { m_controller.emplace(m_instance, m_tag_storage, m_delta_time); }
} // namespace xtag::gui

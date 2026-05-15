#include "app/app.hpp"
#include "app/controller.hpp"
#include "app/main_menu.hpp"
#include "app/main_window.hpp"
#include "clap/parser.hpp"
#include "clap/spec.hpp"
#include "log.hpp"
#include "xtag/build_version.hpp"
#include "xtag/instance.hpp"
#include <cstdlib>

namespace xtag::gui {
namespace {
[[nodiscard]] auto to_scan_info(ScanData const& data) -> ScanInfo {
	auto ret = ScanInfo{.depth = data.depth};
	ret.filter.include_files = data.include_files;
	ret.filter.tag_type |= TagType::Untagged;
	return ret;
}
} // namespace

class App::Controller : public IController, public klib::Pinned {
  public:
	enum class State : std::int8_t { Running, Finished };

	void initialize(Services const& services) final {
		Object::initialize(services);

		m_instance = &services.get<Instance>();

		m_main_menu.initialize(services);
		m_main_window.initialize(services);

		m_state = State::Running;
	}

	void update() final {
		if (ImGui::BeginMainMenuBar()) {
			m_main_menu.update();
			ImGui::EndMainMenuBar();
		}

		auto const& viewport = *ImGui::GetMainViewport();
		ImGui::SetNextWindowPos({0.0f, viewport.WorkPos.y});
		ImGui::SetNextWindowSize(viewport.WorkSize);
		ImGui::Begin("main", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		m_main_window.update();

		ImGui::End();
	}

	[[nodiscard]] auto get_state() const -> State { return m_state; }

	static void set_styles(ImGuiStyle& style) { style.CellPadding = {6.0f, 6.0f}; }

	void on_drop(fs::path const& root) {
		if (!fs::is_directory(root)) { return; }
		m_main_window.scan_data.root = root;
		refresh_root_directory();
	}

	void on_window_close() { shutdown(); }

	void shutdown() final {
		// TODO: cancel async work.
		m_state = State::Finished;
	}

	void refresh_root_directory() final {
		auto& scan_data = m_main_window.scan_data;
		if (scan_data.root.empty()) {
			log.warn("attempt to refresh empty root directory");
			return;
		}

		if (!fs::is_directory(scan_data.root)) {
			log.warn("'{}' is not a directory, resetting", scan_data.root.generic_string());
			scan_data.root.clear();
			return;
		}

		auto const scan_info = to_scan_info(scan_data);
		auto result = m_instance->scan_directory(scan_data.root, scan_info);
		if (!result) {
			log.error("TODO: failed to load directory: '{}'", scan_data.root.generic_string());
			return;
		}

		if (result->entries.empty()) {
			log.error("TODO: internal error: EntryList is unexpectedly empty");
			return;
		}

		m_main_window.set_list(std::move(*result));
	}

  private:
	klib::Ptr<Instance> m_instance{};

	MainMenu m_main_menu{*this};
	MainWindow m_main_window{*this};

	State m_state{State::Running};
};

void App::Deleter::operator()(Controller* ptr) const noexcept { std::default_delete<Controller>{}(ptr); }

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

	Controller::set_styles(ImGui::GetStyle());
}

void App::on_path_drop(std::span<char const* const> paths) {
	if (paths.empty()) { return; }
	m_controller->on_drop(paths.front());
}

void App::update() {
	KLIB_ASSERT(m_controller);
	m_controller->update();
	if (m_controller->get_state() == Controller::State::Finished) { set_should_close_window(true); }
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

void App::initialize() {
	m_services.attach(&m_instance);

	m_controller.reset(new Controller); // NOLINT(cppcoreguidelines-owning-memory)
	m_controller->initialize(m_services);
}
} // namespace xtag::gui

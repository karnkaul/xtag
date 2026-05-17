#pragma once
#include "clap/result.hpp"
#include "gvdi/app.hpp"
#include "service/delta_time.hpp"
#include "service/services.hpp"
#include "ui/controller.hpp"
#include "xtag/instance.hpp"

namespace xtag::gui {
class App : public gvdi::App {
  public:
	[[nodiscard]] auto run(int argc, char const* const* argv) -> int;

  private:
	void stage_create() final;

	auto create_glfw_window() -> GLFWwindow* final;

	void on_path_drop(std::span<char const* const> paths) final;
	void update() final;

	[[nodiscard]] auto parse_args(int argc, char const* const* argv) -> clap::Result;
	void initialize();

	Services m_services{};
	Instance m_instance{};
	DeltaTime m_delta_time{};

	ui::Controller m_controller{};
};
} // namespace xtag::gui

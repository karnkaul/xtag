#pragma once
#include "app/controller.hpp"
#include "clap/result.hpp"
#include "gvdi/app.hpp"
#include "service/services.hpp"
#include "service/signals.hpp"
#include "xtag/instance.hpp"

namespace xtag::gui {
class App : public gvdi::App {
  public:
	[[nodiscard]] auto run(int argc, char const* const* argv) -> int;

  private:
	void stage_create() final;

	void on_path_drop(std::span<char const* const> paths) final;
	void update() final;

	[[nodiscard]] auto parse_args(int argc, char const* const* argv) -> clap::Result;
	void initialize();

	Services m_services{};
	Signals m_signals{};
	Instance m_instance{};

	Controller m_controller{};
};
} // namespace xtag::gui

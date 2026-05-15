#pragma once
#include "clap/result.hpp"
#include "gvdi/app.hpp"
#include "service/services.hpp"
#include "xtag/instance.hpp"
#include <memory>

namespace xtag::gui {
class App : public gvdi::App {
  public:
	[[nodiscard]] auto run(int argc, char const* const* argv) -> int;

  private:
	class Controller;
	struct Deleter {
		void operator()(Controller* ptr) const noexcept;
	};

	void stage_create() final;

	void on_path_drop(std::span<char const* const> paths) final;
	void update() final;

	[[nodiscard]] auto parse_args(int argc, char const* const* argv) -> clap::Result;
	void initialize();

	Services m_services{};
	Instance m_instance{};

	std::unique_ptr<Controller, Deleter> m_controller{};
};
} // namespace xtag::gui

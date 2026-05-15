#include "app/app.hpp"
#include <cstdlib>
#include <exception>
#include <print>

int main(int argc, char** argv) {
	try {
		return xtag::gui::App{}.run(argc, argv);
	} catch (std::exception const& e) {
		std::println(stderr, "PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println(stderr, "PANIC!");
		return EXIT_FAILURE;
	}
}

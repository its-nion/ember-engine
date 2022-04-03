#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "Ember/ember_app.hpp"

int main() {
	ember::EmberApp app{};

	try {
		app.run();
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
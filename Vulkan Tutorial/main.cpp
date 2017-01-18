#include <iostream>
#include <stdexcept>

#include "HelloTriangleApplication.h"

int main() {
	HelloTringleApplication app;

	try {
		app.run();
	}
	catch (const std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		system("PAUSE");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
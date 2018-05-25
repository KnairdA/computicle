#pragma once

#include <chrono>

namespace util {

double millisecondsSince(std::chrono::time_point<std::chrono::high_resolution_clock>& pit) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now() - pit
	).count();
}

}

#include "timer.h"

namespace timer {

std::chrono::time_point<std::chrono::high_resolution_clock> now() {
	return std::chrono::high_resolution_clock::now();
}

double millisecondsSince(
	std::chrono::time_point<std::chrono::high_resolution_clock>& pit) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		now() - pit
	).count();
}

}

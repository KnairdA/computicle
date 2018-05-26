#pragma once

#include <chrono>

namespace timer {

std::chrono::time_point<std::chrono::high_resolution_clock> now();

double millisecondsSince(
	std::chrono::time_point<std::chrono::high_resolution_clock>& pit);

}

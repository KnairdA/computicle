#pragma once

class GlfwGuard {
private:
	bool _good = false;
public:
	GlfwGuard();
	~GlfwGuard();

	bool isGood() const;
};

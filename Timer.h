#pragma once

#include <chrono>

class ChronoTimer {
public:
	bool TimeOver() const;
	void SetTimer(double timeLimit);
	void Start();
	double Elapsed() const;
	double TimeLimit() const;
private:
	std::chrono::system_clock::time_point startT;
	double timeLimit;
};

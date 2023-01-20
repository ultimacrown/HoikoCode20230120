#include "Timer.h"

bool ChronoTimer::TimeOver() const {
	double time = this->Elapsed();
	return this->timeLimit < time;
}
void ChronoTimer::SetTimer(double timeLimit) {
	this->timeLimit = timeLimit;
}
void ChronoTimer::Start() {
	this->startT = std::chrono::system_clock::now();
}
double ChronoTimer::Elapsed() const {
	auto now = std::chrono::system_clock::now();
	return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(now - this->startT).count());
}
double ChronoTimer::TimeLimit() const {
	return timeLimit;
}

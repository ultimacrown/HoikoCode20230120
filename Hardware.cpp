#include "Hardware.h"

#include "framework.h"
#include <algorithm>
#include <vector>
#include <thread>


int HardwareInfo::Physical() {
	DWORD length = 0;

	GetLogicalProcessorInformation(NULL, &length);
	if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) return 0;
	auto size = length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

	std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buf(size);
	if (GetLogicalProcessorInformation(buf.data(), &length) == FALSE) return 0;

	auto pred = [](const SYSTEM_LOGICAL_PROCESSOR_INFORMATION &info) {
		return info.Relationship == RelationProcessorCore;
	};

	return static_cast<int>(std::count_if(buf.begin(), buf.end(), pred));
}
int HardwareInfo::Logical() {
	return std::thread::hardware_concurrency();
}
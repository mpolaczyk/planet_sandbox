
#pragma once

#include <array>
#include <map>
#include <string>

// I removed this from NsightAftermathGpuCrashTracker.h so that can be used in other files without including the whole header...

// Keep four frames worth of marker history
const static uint8_t c_markerFrameHistory = 4;
typedef std::array<std::map<uint64_t, std::string>, c_markerFrameHistory> MarkerMap;
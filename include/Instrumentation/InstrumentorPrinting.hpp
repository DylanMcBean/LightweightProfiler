#pragma once

#include "Instrumentation/InstrumentorUtils.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace InstrumentorPrinting
{
void PrintSessionData(const std::unordered_map<std::string, InstrumentTime> &functionTimes, int64_t totalRuntime);
}
